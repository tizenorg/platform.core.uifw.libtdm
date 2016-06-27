/**************************************************************************
 *
 * libtdm
 *
 * Copyright 2015 Samsung Electronics co., Ltd. All Rights Reserved.
 *
 * Contact: Eunchul Kim <chulspro.kim@samsung.com>,
 *          JinYoung Jeon <jy0.jeon@samsung.com>,
 *          Taeheon Kim <th908.kim@samsung.com>,
 *          YoungJun Cho <yj44.cho@samsung.com>,
 *          SooChan Lim <sc1.lim@samsung.com>,
 *          Boram Park <sc1.lim@samsung.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
**************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <strings.h>

#include "tdm_client.h"
#include "tdm_log.h"
#include "tdm_macro.h"
#include "tdm_list.h"
#include "tdm-client-protocol.h"

int tdm_debug;

typedef struct _tdm_private_client_vblank tdm_private_client_vblank;

typedef struct _tdm_private_client {
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_tdm *tdm;
	struct list_head output_list;

	tdm_private_client_vblank *temp_vblank;
} tdm_private_client;

typedef struct _tdm_private_client_output {
	tdm_private_client *private_client;

	char name[TDM_NAME_LEN];
	struct wl_tdm_output *output;
	int width;
	int height;
	int refresh;
	tdm_output_conn_status connection;
	tdm_output_dpms dpms;
	struct list_head vblank_list;
	struct list_head change_handler_list;

	unsigned int req_id;

	struct list_head link;
} tdm_private_client_output;

struct _tdm_private_client_vblank {
	tdm_private_client_output *private_output;

	struct wl_tdm_vblank *vblank;
	struct list_head wait_list;

	unsigned int sync;
	unsigned int fps;
	int offset;
	unsigned int enable_fake;

	unsigned int started;

	struct list_head link;
};

typedef struct _tdm_client_output_handler_info {
	tdm_private_client_output *private_output;

	tdm_client_output_change_handler func;
	void *user_data;

	struct list_head link;
} tdm_client_output_handler_info;

typedef struct _tdm_client_wait_info {
	tdm_private_client_vblank *private_vblank;

	tdm_client_vblank_handler func;
	void *user_data;

	unsigned int req_id;
	unsigned int req_sec;
	unsigned int req_usec;
	int need_free;

	struct list_head link;
} tdm_client_wait_info;

static void
_tdm_client_vblank_cb_done(void *data, struct wl_tdm_vblank *wl_tdm_vblank,
						   uint32_t req_id, uint32_t sequence, uint32_t tv_sec,
						   uint32_t tv_usec, uint32_t error)
{
	tdm_private_client_vblank *private_vblank = data;
	tdm_client_wait_info *w = NULL, *ww = NULL;

	TDM_RETURN_IF_FAIL(private_vblank != NULL);

	LIST_FOR_EACH_ENTRY_SAFE(w, ww, &private_vblank->wait_list, link) {
		if (w->req_id != req_id)
			continue;

		if (w->func)
			w->func(private_vblank, error, sequence, tv_sec, tv_usec, w->user_data);

		if (w->need_free) {
			LIST_DEL(&w->link);
			free(w);
		} else
			w->need_free = 1;
		return;
	}
}

static const struct wl_tdm_vblank_listener tdm_client_vblank_listener = {
	_tdm_client_vblank_cb_done,
};

static void
_tdm_client_output_destroy(tdm_private_client_output *private_output)
{
	tdm_private_client_vblank *v = NULL, *vv = NULL;
	tdm_client_output_handler_info *h = NULL, *hh = NULL;

	LIST_DEL(&private_output->link);

	LIST_FOR_EACH_ENTRY_SAFE(v, vv, &private_output->vblank_list, link) {
		TDM_ERR("vblanks SHOULD be destroyed first!");
		LIST_DEL(&v->link);
		v->private_output = NULL;
	}

	LIST_FOR_EACH_ENTRY_SAFE(h, hh, &private_output->change_handler_list, link) {
		LIST_DEL(&h->link);
		free(h);
	}

	wl_tdm_output_destroy(private_output->output);

	free(private_output);
}

static void
_tdm_client_output_cb_mode(void *data, struct wl_tdm_output *wl_tdm_output,
						   uint32_t width, uint32_t height, uint32_t refresh)
{
	tdm_private_client_output *private_output = (tdm_private_client_output*)data;

	TDM_RETURN_IF_FAIL(private_output != NULL);

	private_output->width = width;
	private_output->height = height;
	private_output->refresh = refresh;

	TDM_DBG("private_output(%p) wl_tbm_output@%d width(%d) height(%d) refresh(%d)",
			private_output, wl_proxy_get_id((struct wl_proxy*)private_output->output),
			width, height, refresh);
}

static void
_tdm_client_output_cb_connection(void *data, struct wl_tdm_output *wl_tdm_output, uint32_t value)
{
	tdm_private_client_output *private_output = (tdm_private_client_output*)data;
	tdm_client_output_handler_info *h = NULL;
	tdm_value v;

	TDM_RETURN_IF_FAIL(private_output != NULL);

	if (private_output->connection == value)
		return;

	private_output->connection = value;

	TDM_DBG("private_output(%p) wl_tbm_output@%d connection(%d)",
			private_output,
			wl_proxy_get_id((struct wl_proxy*)private_output->output),
			value);

	v.u32 = value;
	LIST_FOR_EACH_ENTRY(h, &private_output->change_handler_list, link) {
		if (h->func)
			h->func(private_output, TDM_OUTPUT_CHANGE_CONNECTION, v, h->user_data);
	}
}

static void
_tdm_client_output_cb_dpms(void *data, struct wl_tdm_output *wl_tdm_output, uint32_t value)
{
	tdm_private_client_output *private_output = (tdm_private_client_output*)data;
	tdm_client_output_handler_info *h = NULL;
	tdm_value v;

	TDM_RETURN_IF_FAIL(private_output != NULL);

	if (private_output->dpms == value)
		return;

	private_output->dpms = value;

	TDM_DBG("private_output(%p) wl_tbm_output@%d dpms(%d)",
			private_output,
			wl_proxy_get_id((struct wl_proxy*)private_output->output),
			value);

	v.u32 = value;
	LIST_FOR_EACH_ENTRY(h, &private_output->change_handler_list, link) {
		if (h->func)
			h->func(private_output, TDM_OUTPUT_CHANGE_DPMS, v, h->user_data);
	}
}

static const struct wl_tdm_output_listener tdm_client_output_listener = {
	_tdm_client_output_cb_mode,
	_tdm_client_output_cb_connection,
	_tdm_client_output_cb_dpms,
};

static void
_tdm_client_cb_global(void *data, struct wl_registry *registry,
					  uint32_t name, const char *interface,
					  uint32_t version)
{
	tdm_private_client *private_client = data;

	if (strcmp(interface, "wl_tdm") == 0) {
		private_client->tdm =
			wl_registry_bind(registry, name, &wl_tdm_interface, version);
		TDM_RETURN_IF_FAIL(private_client->tdm != NULL);

		wl_display_flush(private_client->display);
	}
}

static void
_tdm_client_cb_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
}

static const struct wl_registry_listener tdm_client_registry_listener = {
	_tdm_client_cb_global,
	_tdm_client_cb_global_remove
};

tdm_client*
tdm_client_create(tdm_error *error)
{
	tdm_private_client *private_client;
	const char *debug;

	debug = getenv("TDM_DEBUG");
	if (debug && (strstr(debug, "1")))
		tdm_debug = 1;

	private_client = calloc(1, sizeof *private_client);
	if (!private_client) {
		TDM_ERR("alloc failed");
		if (error)
			*error = TDM_ERROR_OUT_OF_MEMORY;
		return NULL;
	}

	LIST_INITHEAD(&private_client->output_list);

	private_client->display = wl_display_connect("tdm-socket");
	TDM_GOTO_IF_FAIL(private_client->display != NULL, create_failed);

	private_client->registry = wl_display_get_registry(private_client->display);
	TDM_GOTO_IF_FAIL(private_client->registry != NULL, create_failed);

	wl_registry_add_listener(private_client->registry,
							 &tdm_client_registry_listener, private_client);
	wl_display_roundtrip(private_client->display);

	/* check global objects */
	TDM_GOTO_IF_FAIL(private_client->tdm != NULL, create_failed);

	if (error)
		*error = TDM_ERROR_NONE;

	return (tdm_client*)private_client;
create_failed:
	tdm_client_destroy((tdm_client*)private_client);
	if (error)
		*error = TDM_ERROR_OPERATION_FAILED;
	return NULL;
}

void
tdm_client_destroy(tdm_client *client)
{
	tdm_private_client *private_client = (tdm_private_client*)client;
	tdm_private_client_output *o = NULL, *oo = NULL;

	if (!private_client)
		return;

	if (private_client->temp_vblank)
		tdm_client_vblank_destroy(private_client->temp_vblank);

	LIST_FOR_EACH_ENTRY_SAFE(o, oo, &private_client->output_list, link) {
		_tdm_client_output_destroy(o);
	}

	if (private_client->tdm)
		wl_tdm_destroy(private_client->tdm);
	if (private_client->registry)
		wl_registry_destroy(private_client->registry);
	if (private_client->display)
		wl_display_disconnect(private_client->display);

	free(private_client);
}

tdm_error
tdm_client_get_fd(tdm_client *client, int *fd)
{
	tdm_private_client *private_client;

	TDM_RETURN_VAL_IF_FAIL(client != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(fd != NULL, TDM_ERROR_INVALID_PARAMETER);

	private_client = (tdm_private_client*)client;

	*fd = wl_display_get_fd(private_client->display);
	if (*fd < 0)
		return TDM_ERROR_OPERATION_FAILED;

	return TDM_ERROR_NONE;
}

tdm_error
tdm_client_handle_events(tdm_client *client)
{
	tdm_private_client *private_client;

	TDM_RETURN_VAL_IF_FAIL(client != NULL, TDM_ERROR_INVALID_PARAMETER);

	private_client = (tdm_private_client*)client;

	wl_display_dispatch(private_client->display);

	return TDM_ERROR_NONE;
}

typedef struct _tdm_client_vblank_temp {
	tdm_client_vblank_handler2 func;
	void *user_data;
} tdm_client_vblank_temp;

static void
_tdm_client_vblank_handler_temp(tdm_client_vblank *vblank, tdm_error error, unsigned int sequence,
								unsigned int tv_sec, unsigned int tv_usec, void *user_data)
{
	tdm_client_vblank_temp *vblank_temp = user_data;

	TDM_RETURN_IF_FAIL(vblank_temp != NULL);

	if (vblank_temp->func)
		vblank_temp->func(sequence, tv_sec, tv_usec, vblank_temp->user_data);

	free(vblank_temp);
}

tdm_error
tdm_client_wait_vblank(tdm_client *client, char *name,
					   int sw_timer, int interval, int sync,
					   tdm_client_vblank_handler2 func, void *user_data)
{
	tdm_private_client *private_client = (tdm_private_client*)client;
	tdm_client_output *output;
	tdm_client_vblank_temp *vblank_temp;
	tdm_error ret;

	TDM_RETURN_VAL_IF_FAIL(private_client != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(interval > 0, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(func != NULL, TDM_ERROR_INVALID_PARAMETER);

	if (!private_client->temp_vblank) {
		output = tdm_client_get_output(client, name, &ret);
		TDM_RETURN_VAL_IF_FAIL(output != NULL, ret);

		private_client->temp_vblank = tdm_client_output_create_vblank(output, &ret);
		TDM_RETURN_VAL_IF_FAIL(private_client->temp_vblank != NULL, ret);
	}

	tdm_client_vblank_set_enable_fake(private_client->temp_vblank, sw_timer);
	tdm_client_vblank_set_sync(private_client->temp_vblank, sync);

	vblank_temp = calloc(1, sizeof *vblank_temp);
	TDM_RETURN_VAL_IF_FAIL(vblank_temp != NULL, TDM_ERROR_OUT_OF_MEMORY);

	vblank_temp->func = func;
	vblank_temp->user_data = user_data;

	return tdm_client_vblank_wait(private_client->temp_vblank, interval, _tdm_client_vblank_handler_temp, vblank_temp);
}

tdm_client_output*
tdm_client_get_output(tdm_client *client, char *name, tdm_error *error)
{
	tdm_private_client *private_client;
	tdm_private_client_output *private_output = NULL;

	if (error)
		*error = TDM_ERROR_NONE;

	if (!client) {
		TDM_ERR("'!client' failed");
		if (error)
			*error = TDM_ERROR_INVALID_PARAMETER;
		return NULL;
	}

	private_client = (tdm_private_client*)client;

	if (!name)
		name = "primary";

	LIST_FOR_EACH_ENTRY(private_output, &private_client->output_list, link) {
		if (!strncmp(private_output->name, name, TDM_NAME_LEN))
			return (tdm_client_output*)private_output;
	}

	private_output = calloc(1, sizeof *private_output);
	if (!private_output) {
		TDM_ERR("alloc failed");
		if (error)
			*error = TDM_ERROR_OUT_OF_MEMORY;
		return NULL;
	}

	private_output->private_client = private_client;

	snprintf(private_output->name, TDM_NAME_LEN, "%s", name);
	private_output->output = wl_tdm_create_output(private_client->tdm, private_output->name);
	if (!private_output->output) {
		TDM_ERR("couldn't create output resource");
		free(private_output);
		if (error)
			*error = TDM_ERROR_OUT_OF_MEMORY;
		return NULL;
	}

	LIST_INITHEAD(&private_output->vblank_list);
	LIST_INITHEAD(&private_output->change_handler_list);
	LIST_ADDTAIL(&private_output->link, &private_client->output_list);

	wl_tdm_output_add_listener(private_output->output,
							   &tdm_client_output_listener, private_output);
	wl_display_roundtrip(private_client->display);

	return (tdm_client_output*)private_output;
}

tdm_error
tdm_client_output_add_change_handler(tdm_client_output *output,
									 tdm_client_output_change_handler func,
									 void *user_data)
{
	tdm_private_client_output *private_output;
	tdm_client_output_handler_info *h;

	TDM_RETURN_VAL_IF_FAIL(output != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(func != NULL, TDM_ERROR_INVALID_PARAMETER);

	private_output = (tdm_private_client_output*)output;

	h = calloc(1, sizeof *h);
	TDM_RETURN_VAL_IF_FAIL(h != NULL, TDM_ERROR_OUT_OF_MEMORY);

	h->private_output = private_output;
	h->func = func;
	h->user_data = user_data;
	LIST_ADDTAIL(&h->link, &private_output->change_handler_list);

	return TDM_ERROR_NOT_IMPLEMENTED;
}

void
tdm_client_output_remove_change_handler(tdm_client_output *output,
										tdm_client_output_change_handler func,
										void *user_data)
{
	tdm_private_client_output *private_output;
	tdm_client_output_handler_info *h = NULL;

	TDM_RETURN_IF_FAIL(output != NULL);
	TDM_RETURN_IF_FAIL(func != NULL);

	private_output = (tdm_private_client_output*)output;

	LIST_FOR_EACH_ENTRY(h, &private_output->change_handler_list, link) {
		if (h->func != func || h->user_data != user_data)
			continue;

		LIST_DEL(&h->link);
		free(h);
		return;
	}
}

tdm_error
tdm_client_output_get_refresh_rate(tdm_client_output *output, unsigned int *refresh)
{
	tdm_private_client_output *private_output;

	TDM_RETURN_VAL_IF_FAIL(output != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(refresh != NULL, TDM_ERROR_INVALID_PARAMETER);

	private_output = (tdm_private_client_output*)output;

	*refresh = private_output->refresh;

	return TDM_ERROR_NONE;
}

tdm_error
tdm_client_output_get_conn_status(tdm_client_output *output, tdm_output_conn_status *status)
{
	tdm_private_client_output *private_output;

	TDM_RETURN_VAL_IF_FAIL(output != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(status != NULL, TDM_ERROR_INVALID_PARAMETER);

	private_output = (tdm_private_client_output*)output;

	*status = private_output->connection;

	return TDM_ERROR_NONE;
}

tdm_error
tdm_client_output_get_dpms(tdm_client_output *output, tdm_output_dpms *dpms)
{
	tdm_private_client_output *private_output;

	TDM_RETURN_VAL_IF_FAIL(output != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(dpms != NULL, TDM_ERROR_INVALID_PARAMETER);

	private_output = (tdm_private_client_output*)output;

	*dpms = private_output->dpms;

	return TDM_ERROR_NONE;
}

tdm_client_vblank*
tdm_client_output_create_vblank(tdm_client_output *output, tdm_error *error)
{
	tdm_private_client_output *private_output;
	tdm_private_client_vblank *private_vblank;

	if (error)
		*error = TDM_ERROR_NONE;

	if (!output) {
		TDM_ERR("'!output' failed");
		if (error)
			*error = TDM_ERROR_INVALID_PARAMETER;
		return NULL;
	}

	private_output = (tdm_private_client_output*)output;

	private_vblank = calloc(1, sizeof *private_vblank);
	if (!private_vblank) {
		TDM_ERR("alloc failed");
		if (error)
			*error = TDM_ERROR_OUT_OF_MEMORY;
		return NULL;
	}

	private_vblank->private_output = private_output;

	private_vblank->vblank = wl_tdm_output_create_vblank(private_output->output);
	if (!private_vblank->vblank) {
		TDM_ERR("couldn't create vblank resource");
		free(private_vblank);
		if (error)
			*error = TDM_ERROR_OUT_OF_MEMORY;
		return NULL;
	}

	/* initial value */
	private_vblank->fps = private_output->refresh;
	private_vblank->offset = 0;
	private_vblank->enable_fake = 0;

	LIST_INITHEAD(&private_vblank->wait_list);
	LIST_ADDTAIL(&private_vblank->link, &private_output->vblank_list);

	wl_tdm_vblank_add_listener(private_vblank->vblank,
							   &tdm_client_vblank_listener, private_vblank);

	return (tdm_client_vblank*)private_vblank;
}

void
tdm_client_vblank_destroy(tdm_client_vblank *vblank)
{
	tdm_private_client_vblank *private_vblank;
	tdm_client_wait_info *w = NULL, *ww = NULL;

	TDM_RETURN_IF_FAIL(vblank != NULL);

	private_vblank = vblank;
	LIST_DEL(&private_vblank->link);

	LIST_FOR_EACH_ENTRY_SAFE(w, ww, &private_vblank->wait_list, link) {
		LIST_DEL(&w->link);
		free(w);
	}

	wl_tdm_vblank_destroy(private_vblank->vblank);

	free(private_vblank);
}

tdm_error
tdm_client_vblank_set_sync(tdm_client_vblank *vblank, unsigned int sync)
{
	tdm_private_client_vblank *private_vblank;

	TDM_RETURN_VAL_IF_FAIL(vblank != NULL, TDM_ERROR_INVALID_PARAMETER);

	private_vblank = vblank;
	private_vblank->sync = sync;

	return TDM_ERROR_NONE;
}

tdm_error
tdm_client_vblank_set_fps(tdm_client_vblank *vblank, unsigned int fps)
{
	tdm_private_client_vblank *private_vblank;

	TDM_RETURN_VAL_IF_FAIL(vblank != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(fps > 0, TDM_ERROR_INVALID_PARAMETER);

	private_vblank = vblank;
	TDM_RETURN_VAL_IF_FAIL(private_vblank->started == 0, TDM_ERROR_BAD_REQUEST);

	if (private_vblank->fps == fps)
		return TDM_ERROR_NONE;
	private_vblank->fps = fps;

	wl_tdm_vblank_set_fps(private_vblank->vblank, fps);

	return TDM_ERROR_NONE;
}

tdm_error
tdm_client_vblank_set_offset(tdm_client_vblank *vblank, int offset_ms)
{
	tdm_private_client_vblank *private_vblank;

	TDM_RETURN_VAL_IF_FAIL(vblank != NULL, TDM_ERROR_INVALID_PARAMETER);

	private_vblank = vblank;
	TDM_RETURN_VAL_IF_FAIL(private_vblank->started == 0, TDM_ERROR_BAD_REQUEST);

	if (private_vblank->offset == offset_ms)
		return TDM_ERROR_NONE;
	private_vblank->offset = offset_ms;

	wl_tdm_vblank_set_offset(private_vblank->vblank, offset_ms);

	return TDM_ERROR_NONE;
}

tdm_error
tdm_client_vblank_set_enable_fake(tdm_client_vblank *vblank, unsigned int enable_fake)
{
	tdm_private_client_vblank *private_vblank;

	TDM_RETURN_VAL_IF_FAIL(vblank != NULL, TDM_ERROR_INVALID_PARAMETER);

	private_vblank = vblank;

	if (private_vblank->enable_fake == enable_fake)
		return TDM_ERROR_NONE;
	private_vblank->enable_fake = enable_fake;

	wl_tdm_vblank_set_enable_fake(private_vblank->vblank, enable_fake);

	return TDM_ERROR_NONE;
}

tdm_error
tdm_client_vblank_wait(tdm_client_vblank *vblank, unsigned int interval, tdm_client_vblank_handler func, void *user_data)
{
	tdm_private_client *private_client;
	tdm_private_client_output *private_output;
	tdm_private_client_vblank *private_vblank;
	tdm_client_wait_info *w;
	struct timespec tp;
	int ret = 0;

	TDM_RETURN_VAL_IF_FAIL(vblank != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(func != NULL, TDM_ERROR_INVALID_PARAMETER);
	/* can't support "interval 0" and "getting current_msc" things because
	 * there is a socket communication between TDM client and server. It's impossible
	 * to return the current msc or sequence immediately.
	 */
	TDM_RETURN_VAL_IF_FAIL(interval > 0, TDM_ERROR_INVALID_PARAMETER);

	private_vblank = vblank;
	private_output = private_vblank->private_output;
	private_client = private_output->private_client;

	if (!private_vblank->started)
		private_vblank->started = 1;

	if (private_output->dpms != TDM_OUTPUT_DPMS_ON && !private_vblank->enable_fake) {
		TDM_INFO("dpms off");
		return TDM_ERROR_DPMS_OFF;
	}

	w = calloc(1, sizeof *w);
	if (!w) {
		TDM_ERR("alloc failed");
		return TDM_ERROR_OUT_OF_MEMORY;
	}

	w->private_vblank = private_vblank;
	w->func = func;
	w->user_data = user_data;

	LIST_ADDTAIL(&w->link, &private_vblank->wait_list);

	clock_gettime(CLOCK_MONOTONIC, &tp);
	w->req_id = ++private_output->req_id;
	w->req_sec = (unsigned int)tp.tv_sec;
	w->req_usec = (unsigned int)(tp.tv_nsec / 1000);
	w->need_free = (private_vblank->sync) ? 0 : 1;

	wl_tdm_vblank_wait_vblank(private_vblank->vblank, interval, w->req_id, w->req_sec, w->req_usec);

	if (!private_vblank->sync) {
		wl_display_flush(private_client->display);
		return TDM_ERROR_NONE;
	}

	while (ret != -1 && !w->need_free)
		ret = wl_display_dispatch(private_client->display);

	clock_gettime(CLOCK_MONOTONIC, &tp);
	TDM_DBG("block during %d us",
			((unsigned int)(tp.tv_sec * 1000000) + (unsigned int)(tp.tv_nsec / 1000))
			- (w->req_sec * 1000000 + w->req_usec));

	LIST_DEL(&w->link);
	free(w);

	return TDM_ERROR_NONE;
}
