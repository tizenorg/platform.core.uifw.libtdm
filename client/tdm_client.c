/**************************************************************************

libtdm

Copyright 2015 Samsung Electronics co., Ltd. All Rights Reserved.

Contact: Eunchul Kim <chulspro.kim@samsung.com>,
         JinYoung Jeon <jy0.jeon@samsung.com>,
         Taeheon Kim <th908.kim@samsung.com>,
         YoungJun Cho <yj44.cho@samsung.com>,
         SooChan Lim <sc1.lim@samsung.com>,
         Boram Park <sc1.lim@samsung.com>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "tdm_client.h"
#include "tdm_log.h"
#include "tdm_macro.h"
#include "tdm_list.h"
#include "tdm-client-protocol.h"

int tdm_debug;

typedef struct _tdm_private_client {
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_tdm *tdm;

	struct list_head vblank_list;
} tdm_private_client;

typedef struct _tdm_client_vblank_info {
	struct list_head link;
	struct wl_tdm_vblank *vblank;
	tdm_client_vblank_handler func;
	unsigned int req_sec;
	unsigned int req_usec;
	void *user_data;
} tdm_client_vblank_info;

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

static const struct wl_registry_listener tdm_client_registry_listener =
{
    _tdm_client_cb_global,
    _tdm_client_cb_global_remove
};

tdm_client*
tdm_client_create(tdm_client_error *error)
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
			*error = TDM_CLIENT_ERROR_OUT_OF_MEMORY;
		return NULL;
	}

	private_client->display = wl_display_connect("tdm-socket");
	TDM_GOTO_IF_FAIL(private_client->display != NULL, create_failed);

	private_client->registry = wl_display_get_registry(private_client->display);
	TDM_GOTO_IF_FAIL(private_client->registry != NULL, create_failed);

	wl_registry_add_listener(private_client->registry,
	                         &tdm_client_registry_listener, private_client);
	wl_display_roundtrip(private_client->display);

	/* check global objects */
	TDM_GOTO_IF_FAIL(private_client->tdm != NULL, create_failed);

	LIST_INITHEAD(&private_client->vblank_list);

	if (error)
		*error = TDM_CLIENT_ERROR_NONE;

	return (tdm_client*)private_client;
create_failed:
	tdm_client_destroy((tdm_client*)private_client);
	if (error)
		*error = TDM_CLIENT_ERROR_OPERATION_FAILED;
	return NULL;
}

void
tdm_client_destroy(tdm_client *client)
{
	tdm_private_client *private_client = (tdm_private_client*)client;
	tdm_client_vblank_info *v = NULL, *vv = NULL;

	if (!private_client)
		return;

	LIST_FOR_EACH_ENTRY_SAFE(v, vv, &private_client->vblank_list, link) {
		LIST_DEL(&v->link);
		wl_tdm_vblank_destroy(v->vblank);
		free(v);
	}

	if (private_client->tdm)
		wl_tdm_destroy(private_client->tdm);
	if (private_client->registry)
		wl_registry_destroy(private_client->registry);
	if (private_client->display)
		wl_display_disconnect(private_client->display);

	free(private_client);
}

tdm_client_error
tdm_client_get_fd(tdm_client *client, int *fd)
{
	tdm_private_client *private_client;

	TDM_RETURN_VAL_IF_FAIL(client != NULL, TDM_CLIENT_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(fd != NULL, TDM_CLIENT_ERROR_INVALID_PARAMETER);

	private_client = (tdm_private_client*)client;

	*fd = wl_display_get_fd(private_client->display);
	if (*fd < 0)
		return TDM_CLIENT_ERROR_OPERATION_FAILED;

	return TDM_CLIENT_ERROR_NONE;
}

tdm_client_error
tdm_client_handle_events(tdm_client *client)
{
	tdm_private_client *private_client;

	TDM_RETURN_VAL_IF_FAIL(client != NULL, TDM_CLIENT_ERROR_INVALID_PARAMETER);

	private_client = (tdm_private_client*)client;

	wl_display_dispatch(private_client->display);

	return TDM_CLIENT_ERROR_NONE;
}

static void
_tdm_client_cb_vblank_done(void *data, struct wl_tdm_vblank *vblank,
                           uint32_t sequence, uint32_t tv_sec, uint32_t tv_usec)
{
	tdm_client_vblank_info *vblank_info = (tdm_client_vblank_info*)data;

	TDM_RETURN_IF_FAIL(vblank_info != NULL);

	if (vblank_info->vblank != vblank)
		TDM_NEVER_GET_HERE();

	TDM_DBG("vblank_info(%p) wl_tbm_vblank@%d", vblank_info,
	        wl_proxy_get_id((struct wl_proxy *)vblank));

	if (vblank_info->func) {
		vblank_info->func(sequence, tv_sec, tv_usec, vblank_info->user_data);
	}

	LIST_DEL(&vblank_info->link);
	free(vblank_info);
}

static const struct wl_tdm_vblank_listener tdm_client_vblank_listener = {
	_tdm_client_cb_vblank_done,
};

tdm_client_error
tdm_client_wait_vblank(tdm_client *client, char *name,
                       int sw_timer, int interval, int sync,
                       tdm_client_vblank_handler func, void *user_data)
{
	tdm_private_client *private_client = (tdm_private_client*)client;
	tdm_client_vblank_info *vblank_info;
	struct timespec tp;

	TDM_RETURN_VAL_IF_FAIL(name != NULL, TDM_CLIENT_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(interval > 0, TDM_CLIENT_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(func != NULL, TDM_CLIENT_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(private_client != NULL, TDM_CLIENT_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(private_client->tdm != NULL, TDM_CLIENT_ERROR_INVALID_PARAMETER);

	vblank_info = calloc(1, sizeof *vblank_info);
	if (!vblank_info) {
		TDM_ERR("alloc failed");
		return TDM_CLIENT_ERROR_OUT_OF_MEMORY;
	}

	clock_gettime(CLOCK_MONOTONIC, &tp);

	vblank_info->req_sec = (unsigned int)tp.tv_sec;
	vblank_info->req_usec = (unsigned int)(tp.tv_nsec/1000L);

	vblank_info->vblank =
		wl_tdm_wait_vblank(private_client->tdm, name, sw_timer, interval,
		                   vblank_info->req_sec, vblank_info->req_usec);
	if (!vblank_info->vblank) {
		TDM_ERR("couldn't create vblank resource");
		free(vblank_info);
		return TDM_CLIENT_ERROR_OUT_OF_MEMORY;
	}

	TDM_DBG("vblank_info(%p) wl_tbm_vblank@%d", vblank_info,
	        wl_proxy_get_id((struct wl_proxy *)vblank_info->vblank));

	wl_tdm_vblank_add_listener(vblank_info->vblank,
	                           &tdm_client_vblank_listener, vblank_info);

	vblank_info->func = func;
	vblank_info->user_data = user_data;
	LIST_ADDTAIL(&vblank_info->link, &private_client->vblank_list);

	if (sync)
		wl_display_roundtrip(private_client->display);
	else
		wl_display_flush(private_client->display);

	return TDM_CLIENT_ERROR_NONE;
}
