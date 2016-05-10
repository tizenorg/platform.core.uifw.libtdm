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

#include "tdm.h"
#include "tdm_private.h"
#include "tdm_list.h"
#include "tdm-server-protocol.h"

/* CAUTION:
 * - tdm server doesn't care about thread things.
 * - DO NOT use the TDM internal functions here.
 *     However, the internal function which does lock/unlock the mutex of
 *     private_display in itself can be called.
 * - DO NOT use the tdm_private_display structure here.
 */

struct _tdm_private_server {
	tdm_private_loop *private_loop;
	struct list_head client_list;
	struct list_head vblank_list;
};

typedef struct _tdm_server_client_info {
	struct list_head link;
	tdm_private_server *private_server;
	struct wl_resource *resource;

	tdm_output *vblank_output;
	double vblank_gap;
	unsigned int last_tv_sec;
	unsigned int last_tv_usec;
} tdm_server_client_info;

typedef struct _tdm_server_vblank_info {
	struct list_head link;
	tdm_server_client_info *client_info;
	struct wl_resource *resource;

	unsigned int req_sec;
	unsigned int req_usec;

	tdm_event_loop_source *timer_source;
	unsigned int timer_target_sec;
	unsigned int timer_target_usec;
} tdm_server_vblank_info;

static tdm_private_server *keep_private_server;
static int tdm_debug_server;

static void
_tdm_server_send_done(tdm_server_vblank_info *vblank_info, unsigned int sequence,
                      unsigned int tv_sec, unsigned int tv_usec);

static tdm_error
_tdm_server_cb_timer(void *user_data)
{
	tdm_server_vblank_info *vblank_info = (tdm_server_vblank_info*)user_data;

	_tdm_server_send_done(vblank_info, 0,
	                      vblank_info->timer_target_sec,
	                      vblank_info->timer_target_usec);

	return TDM_ERROR_NONE;
}

static tdm_error
_tdm_server_update_timer(tdm_server_vblank_info *vblank_info, int interval)
{
	tdm_server_client_info *client_info = vblank_info->client_info;
	tdm_private_server *private_server = client_info->private_server;
	tdm_private_loop *private_loop = private_server->private_loop;
	unsigned long last, prev_req, req, curr, next;
	unsigned int ms_delay;
	tdm_error ret;

	ret = tdm_display_lock(private_loop->dpy);
	TDM_RETURN_VAL_IF_FAIL(ret == TDM_ERROR_NONE, ret);

	if (!vblank_info->timer_source) {
		vblank_info->timer_source =
			tdm_event_loop_add_timer_handler(private_loop->dpy,
			                                 _tdm_server_cb_timer,
			                                 vblank_info,
			                                 NULL);
		if (!vblank_info->timer_source) {
			TDM_ERR("couldn't add timer");
			tdm_display_unlock(private_loop->dpy);
			return TDM_ERROR_OPERATION_FAILED;
		}
	}

	last = (unsigned long)client_info->last_tv_sec * 1000000 + client_info->last_tv_usec;
	req = (unsigned long)vblank_info->req_sec * 1000000 + vblank_info->req_usec;
	curr = tdm_helper_get_time_in_micros();

	prev_req = last + (unsigned int)((req - last) / client_info->vblank_gap) * client_info->vblank_gap;
	next = prev_req + (unsigned long)(client_info->vblank_gap * interval);

	while (next < curr)
		next += (unsigned long)client_info->vblank_gap;

	TDM_DBG("last(%.6lu) req(%.6lu) curr(%.6lu) prev_req(%.6lu) next(%.6lu)",
	        last, req, curr, prev_req, next);

	ms_delay = (unsigned int)ceil((double)(next - curr) / 1000);
	if (ms_delay == 0)
		ms_delay = 1;

	TDM_DBG("delay(%lu) ms_delay(%d)", next - curr, ms_delay);

	ret = tdm_event_loop_source_timer_update(vblank_info->timer_source, ms_delay);
	if (ret != TDM_ERROR_NONE) {
		tdm_event_loop_source_remove(vblank_info->timer_source);
		vblank_info->timer_source = NULL;
		tdm_display_unlock(private_loop->dpy);
		TDM_ERR("couldn't update timer");
		return TDM_ERROR_OPERATION_FAILED;
	}

	TDM_DBG("timer tick: %d us", (int)(next - last));

	vblank_info->timer_target_sec = next / 1000000;
	vblank_info->timer_target_usec = next % 1000000;

	tdm_display_unlock(private_loop->dpy);

	return TDM_ERROR_NONE;
}

static void
_tdm_server_send_done(tdm_server_vblank_info *vblank_info, unsigned int sequence,
                      unsigned int tv_sec, unsigned int tv_usec)
{
	tdm_server_vblank_info *found;
	tdm_server_client_info *client_info;
	unsigned long vtime = (tv_sec * 1000000) + tv_usec;
	unsigned long curr = tdm_helper_get_time_in_micros();

	if (!keep_private_server)
		return;

	LIST_FIND_ITEM(vblank_info, &keep_private_server->vblank_list,
	               tdm_server_vblank_info, link, found);
	if (!found) {
		TDM_DBG("vblank_info(%p) is destroyed", vblank_info);
		return;
	}

	client_info = vblank_info->client_info;
	client_info->last_tv_sec = tv_sec;
	client_info->last_tv_usec = tv_usec;

	TDM_DBG("wl_tdm_vblank@%d done. tv(%lu) curr(%lu)",
	        wl_resource_get_id(vblank_info->resource), vtime, curr);

	if (tdm_debug_server) {
		if (curr - vtime > 1000) /* 1ms */
			TDM_WRN("delay: %d us", (int)(curr - vtime));
	}

	wl_tdm_vblank_send_done(vblank_info->resource, sequence, tv_sec, tv_usec);
	wl_resource_destroy(vblank_info->resource);
}

static void
_tdm_server_cb_output_vblank(tdm_output *output, unsigned int sequence,
                             unsigned int tv_sec, unsigned int tv_usec,
                             void *user_data)
{
	tdm_server_vblank_info *vblank_info = (tdm_server_vblank_info*)user_data;

	_tdm_server_send_done(vblank_info, sequence, tv_sec, tv_usec);
}

static void
destroy_vblank_callback(struct wl_resource *resource)
{
	tdm_server_vblank_info *vblank_info = wl_resource_get_user_data(resource);

	if (vblank_info->timer_source) {
		tdm_private_server *private_server = vblank_info->client_info->private_server;

		tdm_display_lock(private_server->private_loop->dpy);
		tdm_event_loop_source_remove(vblank_info->timer_source);
		tdm_display_unlock(private_server->private_loop->dpy);
	}

	LIST_DEL(&vblank_info->link);
	free(vblank_info);
}

static void
_tdm_server_client_cb_destroy(struct wl_client *client, struct wl_resource *resource)
{
	wl_resource_destroy(resource);
}

static void
_tdm_server_client_cb_wait_vblank(struct wl_client *client,
                                  struct wl_resource *resource,
                                  uint32_t id, const char *name,
                                  int32_t sw_timer, int32_t interval,
                                  uint32_t req_sec, uint32_t req_usec)
{
	tdm_server_client_info *client_info = wl_resource_get_user_data(resource);
	tdm_private_server *private_server = client_info->private_server;
	tdm_private_loop *private_loop = private_server->private_loop;
	tdm_server_vblank_info *vblank_info;
	struct wl_resource *vblank_resource;
	tdm_output *found = NULL;
	tdm_output_dpms dpms_value = TDM_OUTPUT_DPMS_ON;
	tdm_error ret;
	const char *model;

	TDM_DBG("The tdm client requests vblank");

	if (tdm_debug_server) {
		unsigned long curr = tdm_helper_get_time_in_micros();
		unsigned long reqtime = (req_sec * 1000000) + req_usec;
		if (curr - reqtime > 1000) /* 1ms */
			TDM_WRN("delay(req): %d us", (int)(curr - reqtime));
	}

	if (client_info->vblank_output) {
		model = NULL;
		ret = tdm_output_get_model_info(client_info->vblank_output, NULL, &model, NULL);
		if (model && !strncmp(model, name, TDM_NAME_LEN))
			found = client_info->vblank_output;
	}

	if (!found) {
		int count = 0, i;

		tdm_display_get_output_count(private_loop->dpy, &count);

		for (i = 0; i < count; i++) {
			tdm_output *output= tdm_display_get_output(private_loop->dpy, i, NULL);
			tdm_output_conn_status status;

			ret = tdm_output_get_conn_status(output, &status);
			if (ret || status == TDM_OUTPUT_CONN_STATUS_DISCONNECTED)
				continue;

			model = NULL;
			ret = tdm_output_get_model_info(output, NULL, &model, NULL);
			if (ret || !model)
				continue;

			if (strncmp(model, name, TDM_NAME_LEN))
				continue;

			found = output;
			break;
		}
	}

	if (!found) {
		wl_resource_post_error(resource, WL_TDM_CLIENT_ERROR_INVALID_NAME,
		                       "There is no '%s' output", name);
		TDM_ERR("There is no '%s' output", name);
		return;
	}

	if (client_info->vblank_output != found) {
		const tdm_output_mode *mode = NULL;

		client_info->vblank_output = found;

		tdm_output_get_mode(client_info->vblank_output, &mode);
		if (!mode || mode->vrefresh <= 0) {
			wl_resource_post_error(resource, WL_TDM_CLIENT_ERROR_OPERATION_FAILED,
			                       "couldn't get mode of %s", name);
			TDM_ERR("couldn't get mode of %s", name);
			return;
		}

		client_info->vblank_gap = (double)1000000 / mode->vrefresh;
		TDM_INFO("vblank_gap(%.6lf)", client_info->vblank_gap);
	}

	tdm_output_get_dpms(found, &dpms_value);

	if (dpms_value != TDM_OUTPUT_DPMS_ON && !sw_timer) {
		wl_resource_post_error(resource, WL_TDM_CLIENT_ERROR_DPMS_OFF,
		                       "dpms '%s'", tdm_get_dpms_str(dpms_value));
		TDM_ERR("dpms '%s'", tdm_get_dpms_str(dpms_value));
		return;
	}

	vblank_info = calloc(1, sizeof *vblank_info);
	if (!vblank_info) {
		wl_resource_post_no_memory(resource);
		TDM_ERR("alloc failed");
		return;
	}

	TDM_DBG("wl_tdm_vblank@%d output(%s) interval(%d)", id, name, interval);

	vblank_resource =
		wl_resource_create(client, &wl_tdm_vblank_interface,
		                   wl_resource_get_version(resource), id);
	if (!vblank_resource) {
		wl_resource_post_no_memory(resource);
		TDM_ERR("wl_resource_create failed");
		goto free_info;
	}

	vblank_info->resource = vblank_resource;
	vblank_info->client_info = client_info;
	vblank_info->req_sec = req_sec;
	vblank_info->req_usec = req_usec;

	if (dpms_value == TDM_OUTPUT_DPMS_ON) {
		ret = tdm_output_wait_vblank(found, interval, 0,
		                             _tdm_server_cb_output_vblank, vblank_info);
		if (ret != TDM_ERROR_NONE) {
			wl_resource_post_error(resource, WL_TDM_CLIENT_ERROR_OPERATION_FAILED,
			                       "couldn't wait vblank for %s", name);
			TDM_ERR("couldn't wait vblank for %s", name);
			goto destroy_resource;
		}
	} else if (sw_timer) {
		ret = _tdm_server_update_timer(vblank_info, interval);
		if (ret != TDM_ERROR_NONE) {
			wl_resource_post_error(resource, WL_TDM_CLIENT_ERROR_OPERATION_FAILED,
			                       "couldn't update timer for %s", name);
			TDM_ERR("couldn't update timer for %s", name);
			goto destroy_resource;
		}
	} else {
		wl_resource_post_error(resource, WL_TDM_CLIENT_ERROR_OPERATION_FAILED,
		                       "bad implementation");
		TDM_NEVER_GET_HERE();
		goto destroy_resource;
	}

	wl_resource_set_implementation(vblank_resource, NULL, vblank_info,
	                               destroy_vblank_callback);

	LIST_ADDTAIL(&vblank_info->link, &private_server->vblank_list);
	return;
destroy_resource:
	wl_resource_destroy(vblank_resource);
free_info:
	free(vblank_info);
}

static const struct wl_tdm_client_interface tdm_client_implementation = {
	_tdm_server_client_cb_destroy,
	_tdm_server_client_cb_wait_vblank,
};

static void
destroy_client_callback(struct wl_resource *resource)
{
	tdm_server_client_info *client_info = wl_resource_get_user_data(resource);
	LIST_DEL(&client_info->link);
	free(client_info);
}

static void
_tdm_server_cb_create_client(struct wl_client *client,
                             struct wl_resource *resource, uint32_t id)
{
	tdm_private_server *private_server = wl_resource_get_user_data(resource);
	tdm_server_client_info *client_info;
	struct wl_resource *client_resource;

	client_info = calloc(1, sizeof *client_info);
	if (!client_info) {
		wl_resource_post_no_memory(resource);
		TDM_ERR("alloc failed");
		return;
	}

	client_resource =
		wl_resource_create(client, &wl_tdm_client_interface,
		                   wl_resource_get_version(resource), id);
	if (!client_resource) {
		wl_resource_post_no_memory(resource);
		free(client_info);
		TDM_ERR("wl_resource_create failed");
		return;
	}

	client_info->private_server = private_server;
	client_info->resource = client_resource;

	wl_resource_set_implementation(client_resource, &tdm_client_implementation,
	                               client_info, destroy_client_callback);

	LIST_ADDTAIL(&client_info->link, &private_server->client_list);
}

static const struct wl_tdm_interface tdm_implementation = {
	_tdm_server_cb_create_client,
};

static void
_tdm_server_bind(struct wl_client *client, void *data,
                 uint32_t version, uint32_t id)
{
	struct wl_resource *resource;

	resource = wl_resource_create(client, &wl_tdm_interface, version, id);
	if (!resource) {
		wl_client_post_no_memory(client);
		return;
	}

	TDM_DBG("tdm server binding");

	wl_resource_set_implementation(resource, &tdm_implementation, data, NULL);
}

INTERN tdm_error
tdm_server_init(tdm_private_loop *private_loop)
{
	tdm_private_server *private_server;
	const char *debug;

	debug = getenv("TDM_DEBUG_SERVER");
	if (debug && (strstr(debug, "1")))
		tdm_debug_server = 1;

	if (private_loop->private_server)
		return TDM_ERROR_NONE;

	TDM_RETURN_VAL_IF_FAIL(private_loop, TDM_ERROR_OPERATION_FAILED);
	TDM_RETURN_VAL_IF_FAIL(private_loop->wl_display, TDM_ERROR_OPERATION_FAILED);

	if(wl_display_add_socket(private_loop->wl_display, "tdm-socket")) {
		TDM_ERR("createing a tdm-socket failed");
		return TDM_ERROR_OPERATION_FAILED;
	}

	private_server = calloc(1, sizeof *private_server);
	if (!private_server) {
		TDM_ERR("alloc failed");
		return TDM_ERROR_OUT_OF_MEMORY;
	}

	LIST_INITHEAD(&private_server->client_list);
	LIST_INITHEAD(&private_server->vblank_list);

	if (!wl_global_create(private_loop->wl_display, &wl_tdm_interface, 1,
	                      private_server, _tdm_server_bind)) {
		TDM_ERR("creating a global resource failed");
		free(private_server);
		return TDM_ERROR_OUT_OF_MEMORY;
	}

	private_server->private_loop = private_loop;
	private_loop->private_server = private_server;
	keep_private_server = private_server;

	return TDM_ERROR_NONE;
}

INTERN void
tdm_server_deinit(tdm_private_loop *private_loop)
{
	tdm_server_vblank_info *v = NULL, *vv = NULL;
	tdm_server_client_info *c = NULL, *cc = NULL;
	tdm_private_server *private_server;

	if (!private_loop->private_server)
		return;

	private_server = private_loop->private_server;

	LIST_FOR_EACH_ENTRY_SAFE(v, vv, &private_server->vblank_list, link) {
		wl_resource_destroy(v->resource);
	}

	LIST_FOR_EACH_ENTRY_SAFE(c, cc, &private_server->client_list, link) {
		wl_resource_destroy(c->resource);
	}

	free(private_server);
	private_loop->private_server = NULL;
	keep_private_server = NULL;
}
