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
 * - The callback function things can be called in main thread.
 */

struct _tdm_private_server {
	tdm_private_loop *private_loop;
	struct list_head output_list;
	struct list_head wait_list;
};

typedef struct _tdm_server_output_info {
	struct list_head link;
	tdm_private_server *private_server;
	struct wl_resource *resource;
	tdm_output *output;
	struct list_head vblank_list;
} tdm_server_output_info;

typedef struct _tdm_server_vblank_info {
	struct list_head link;
	tdm_server_output_info *output_info;
	struct wl_resource *resource;

	tdm_vblank *vblank;
} tdm_server_vblank_info;

typedef struct _tdm_server_wait_info {
	struct list_head link;
	tdm_server_vblank_info *vblank_info;

	unsigned int req_id;
} tdm_server_wait_info;

static tdm_private_server *keep_private_server;

static void destroy_wait(tdm_server_wait_info *wait_info);

static tdm_output*
_tdm_server_find_output(tdm_private_server *private_server, const char *name)
{
	tdm_private_loop *private_loop = private_server->private_loop;
	tdm_output *found = NULL;

	if (!strncasecmp(name, "primary", 7) || !strncasecmp(name, "default", 7))
		found = tdm_display_get_output(private_loop->dpy, 0, NULL);

	if (!found) {
		int count = 0, i;

		tdm_display_get_output_count(private_loop->dpy, &count);

		for (i = 0; i < count; i++) {
			tdm_output *output = tdm_display_get_output(private_loop->dpy, i, NULL);
			tdm_output_conn_status status;
			const char *model = NULL;
			tdm_error ret;

			ret = tdm_output_get_conn_status(output, &status);
			if (ret || status == TDM_OUTPUT_CONN_STATUS_DISCONNECTED)
				continue;

			ret = tdm_output_get_model_info(output, NULL, &model, NULL);
			if (ret || !model)
				continue;

			if (strncmp(model, name, TDM_NAME_LEN))
				continue;

			found = output;
			break;
		}
	}

	return found;
}

static void
_tdm_server_send_done(tdm_server_wait_info *wait_info, tdm_error error,
					  unsigned int sequence, unsigned int tv_sec, unsigned int tv_usec)
{
	tdm_server_wait_info *found;
	tdm_server_vblank_info *vblank_info;

	if (!keep_private_server)
		return;

	LIST_FIND_ITEM(wait_info, &keep_private_server->wait_list,
				   tdm_server_wait_info, link, found);
	if (!found) {
		TDM_DBG("wait_info(%p) is destroyed", wait_info);
		return;
	}

	if (tdm_debug_module & TDM_DEBUG_VBLANK)
		TDM_INFO("req_id(%d) done", wait_info->req_id);

	vblank_info = wait_info->vblank_info;
	wl_tdm_vblank_send_done(vblank_info->resource, wait_info->req_id,
							sequence, tv_sec, tv_usec, error);
	destroy_wait(wait_info);
}

static void
_tdm_server_cb_vblank(tdm_vblank *vblank, tdm_error error, unsigned int sequence,
					  unsigned int tv_sec, unsigned int tv_usec, void *user_data)
{
	_tdm_server_send_done((tdm_server_wait_info*)user_data, error, sequence, tv_sec, tv_usec);
}

static void
_tdm_server_cb_output_change(tdm_output *output, tdm_output_change_type type,
							 tdm_value value, void *user_data)
{
	tdm_server_output_info *output_info = user_data;

	TDM_RETURN_IF_FAIL(output_info != NULL);

	switch (type) {
	case TDM_OUTPUT_CHANGE_DPMS:
		wl_tdm_output_send_dpms(output_info->resource, value.u32);
		break;
	case TDM_OUTPUT_CHANGE_CONNECTION:
		wl_tdm_output_send_connection(output_info->resource, value.u32);
		break;
	default:
		break;
	}
}

static void
destroy_wait(tdm_server_wait_info *wait_info)
{
	LIST_DEL(&wait_info->link);
	free(wait_info);
}

static void
destroy_vblank_callback(struct wl_resource *resource)
{
	tdm_server_vblank_info *vblank_info = wl_resource_get_user_data(resource);
	tdm_server_wait_info *w = NULL, *ww = NULL;

	TDM_RETURN_IF_FAIL(vblank_info != NULL);

	if (vblank_info->vblank)
		tdm_vblank_destroy(vblank_info->vblank);

	LIST_FOR_EACH_ENTRY_SAFE(w, ww, &keep_private_server->wait_list, link) {
		if (w->vblank_info == vblank_info) {
			destroy_wait(w);
		}
	}

	LIST_DEL(&vblank_info->link);
	free(vblank_info);
}

static void
_tdm_server_vblank_cb_destroy(struct wl_client *client, struct wl_resource *resource)
{
	wl_resource_destroy(resource);
}

static void
_tdm_server_vblank_cb_set_fps(struct wl_client *client, struct wl_resource *resource, uint32_t fps)
{
	tdm_server_vblank_info *vblank_info = wl_resource_get_user_data(resource);

	tdm_vblank_set_fps(vblank_info->vblank, fps);
}

static void
_tdm_server_vblank_cb_set_offset(struct wl_client *client, struct wl_resource *resource, int32_t offset)
{
	tdm_server_vblank_info *vblank_info = wl_resource_get_user_data(resource);

	tdm_vblank_set_offset(vblank_info->vblank, offset);
}

static void
_tdm_server_vblank_cb_set_enable_fake(struct wl_client *client, struct wl_resource *resource, uint32_t enable_fake)
{
	tdm_server_vblank_info *vblank_info = wl_resource_get_user_data(resource);

	tdm_vblank_set_enable_fake(vblank_info->vblank, enable_fake);
}

static void
_tdm_server_vblank_cb_wait_vblank(struct wl_client *client, struct wl_resource *resource,
								  uint32_t interval, uint32_t req_id, uint32_t req_sec, uint32_t req_usec)
{
	tdm_server_vblank_info *vblank_info = wl_resource_get_user_data(resource);
	tdm_server_output_info *output_info = vblank_info->output_info;
	tdm_private_server *private_server = output_info->private_server;
	tdm_server_wait_info *wait_info;
	tdm_error ret;

	wait_info = calloc(1, sizeof *wait_info);
	if (!wait_info) {
		TDM_ERR("alloc failed");
		ret = TDM_ERROR_OUT_OF_MEMORY;
		goto wait_failed;
	}

	LIST_ADDTAIL(&wait_info->link, &private_server->wait_list);
	wait_info->vblank_info = vblank_info;
	wait_info->req_id = req_id;

	if (tdm_debug_module & TDM_DEBUG_VBLANK)
		TDM_INFO("req_id(%d) wait", req_id);

	ret = tdm_vblank_wait(vblank_info->vblank, req_sec, req_usec, interval, _tdm_server_cb_vblank, wait_info);
	TDM_GOTO_IF_FAIL(ret == TDM_ERROR_NONE, wait_failed);

	return;
wait_failed:
	wl_tdm_vblank_send_done(vblank_info->resource, req_id, 0, 0, 0, ret);
}

static const struct wl_tdm_vblank_interface tdm_vblank_implementation = {
	_tdm_server_vblank_cb_destroy,
	_tdm_server_vblank_cb_set_fps,
	_tdm_server_vblank_cb_set_offset,
	_tdm_server_vblank_cb_set_enable_fake,
	_tdm_server_vblank_cb_wait_vblank,
};

static void
_tdm_server_output_cb_destroy(struct wl_client *client, struct wl_resource *resource)
{
	wl_resource_destroy(resource);
}

static void
_tdm_server_output_cb_create_vblank(struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
	tdm_server_output_info *output_info = wl_resource_get_user_data(resource);
	tdm_private_server *private_server = output_info->private_server;
	tdm_private_loop *private_loop = private_server->private_loop;
	struct wl_resource *vblank_resource;
	tdm_vblank *vblank;
	tdm_server_vblank_info *vblank_info;

	vblank_resource =
		wl_resource_create(client, &wl_tdm_vblank_interface,
						   wl_resource_get_version(resource), id);
	if (!vblank_resource) {
		wl_resource_post_no_memory(resource);
		TDM_ERR("wl_resource_create failed");
		return;
	}

	vblank = tdm_vblank_create(private_loop->dpy, output_info->output, NULL);
	if (!vblank) {
		wl_resource_post_no_memory(resource);
		wl_resource_destroy(vblank_resource);
		TDM_ERR("tdm_vblank_create failed");
		return;
	}

	vblank_info = calloc(1, sizeof *vblank_info);
	if (!vblank_info) {
		wl_resource_post_no_memory(resource);
		wl_resource_destroy(vblank_resource);
		tdm_vblank_destroy(vblank);
		TDM_ERR("alloc failed");
		return;
	}

	LIST_ADDTAIL(&vblank_info->link, &output_info->vblank_list);
	vblank_info->output_info = output_info;
	vblank_info->resource = vblank_resource;
	vblank_info->vblank = vblank;

	wl_resource_set_implementation(vblank_resource, &tdm_vblank_implementation,
								   vblank_info, destroy_vblank_callback);

	return;
}

static const struct wl_tdm_output_interface tdm_output_implementation = {
	_tdm_server_output_cb_destroy,
	_tdm_server_output_cb_create_vblank,
};

static void
destroy_output_callback(struct wl_resource *resource)
{
	tdm_server_output_info *output_info = wl_resource_get_user_data(resource);
	tdm_server_vblank_info *v = NULL, *vv = NULL;

	TDM_RETURN_IF_FAIL(output_info != NULL);

	tdm_output_remove_change_handler(output_info->output,
									 _tdm_server_cb_output_change, output_info);

	LIST_FOR_EACH_ENTRY_SAFE(v, vv, &output_info->vblank_list, link) {
		wl_resource_destroy(v->resource);
	}

	LIST_DEL(&output_info->link);
	free(output_info);
}

static void
_tdm_server_cb_create_output(struct wl_client *client, struct wl_resource *resource,
							 const char *name, uint32_t id)
{
	tdm_private_server *private_server = wl_resource_get_user_data(resource);
	tdm_server_output_info *output_info;
	struct wl_resource *output_resource = NULL;
	tdm_output *output;
	const tdm_output_mode *mode = NULL;
	tdm_output_dpms dpms_value;
	tdm_output_conn_status status;

	output = _tdm_server_find_output(private_server, name);
	if (!output) {
		TDM_ERR("There is no '%s' output", name);
		wl_resource_post_error(resource, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "There is no '%s' output", name);
		return;
	}

	tdm_output_get_mode(output, &mode);
	if (!mode) {
		TDM_ERR("no mode for '%s' output", name);
		wl_resource_post_error(resource, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "no mode for '%s' output", name);
		return;
	}

	tdm_output_get_dpms(output, &dpms_value);
	tdm_output_get_conn_status(output, &status);

	output_resource =
		wl_resource_create(client, &wl_tdm_output_interface,
						   wl_resource_get_version(resource), id);
	if (!output_resource) {
		wl_resource_post_no_memory(resource);
		TDM_ERR("wl_resource_create failed");
		return;
	}

	output_info = calloc(1, sizeof * output_info);
	if (!output_info) {
		wl_resource_post_no_memory(resource);
		wl_resource_destroy(output_resource);
		TDM_ERR("alloc failed");
		return;
	}

	LIST_ADDTAIL(&output_info->link, &private_server->output_list);
	output_info->private_server = private_server;
	output_info->resource = output_resource;
	output_info->output = output;
	LIST_INITHEAD(&output_info->vblank_list);

	tdm_output_add_change_handler(output, _tdm_server_cb_output_change, output_info);

	wl_resource_set_implementation(output_resource, &tdm_output_implementation,
								   output_info, destroy_output_callback);

	wl_tdm_output_send_mode(output_resource, mode->hdisplay, mode->vdisplay, mode->vrefresh);
	wl_tdm_output_send_dpms(output_resource, dpms_value);
	wl_tdm_output_send_connection(output_resource, status);
}

static void
_tdm_server_cb_debug(struct wl_client *client, struct wl_resource *resource, const char *options)
{
	tdm_private_server *private_server = wl_resource_get_user_data(resource);
	tdm_private_loop *private_loop = private_server->private_loop;

	char message[TDM_SERVER_REPLY_MSG_LEN];
	int size = sizeof(message);

	tdm_dbg_server_command(private_loop->dpy, options, message, &size);
	wl_tdm_send_debug_done(resource, message);
}

static const struct wl_tdm_interface tdm_implementation = {
	_tdm_server_cb_create_output,
	_tdm_server_cb_debug,
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

	wl_resource_set_implementation(resource, &tdm_implementation, data, NULL);
}

INTERN tdm_error
tdm_server_init(tdm_private_loop *private_loop)
{
	tdm_private_server *private_server;

	if (private_loop->private_server)
		return TDM_ERROR_NONE;

	TDM_RETURN_VAL_IF_FAIL(private_loop, TDM_ERROR_OPERATION_FAILED);
	TDM_RETURN_VAL_IF_FAIL(private_loop->wl_display, TDM_ERROR_OPERATION_FAILED);

	if (wl_display_add_socket(private_loop->wl_display, "tdm-socket")) {
		TDM_ERR("createing a tdm-socket failed");
		return TDM_ERROR_OPERATION_FAILED;
	}

	private_server = calloc(1, sizeof * private_server);
	if (!private_server) {
		TDM_ERR("alloc failed");
		return TDM_ERROR_OUT_OF_MEMORY;
	}

	LIST_INITHEAD(&private_server->output_list);
	LIST_INITHEAD(&private_server->wait_list);

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
	tdm_server_output_info *o = NULL, *oo = NULL;
	tdm_server_wait_info *w = NULL, *ww = NULL;
	tdm_private_server *private_server;

	if (!private_loop->private_server)
		return;

	private_server = private_loop->private_server;

	LIST_FOR_EACH_ENTRY_SAFE(w, ww, &private_server->wait_list, link) {
		destroy_wait(w);
	}

	LIST_FOR_EACH_ENTRY_SAFE(o, oo, &private_server->output_list, link) {
		wl_resource_destroy(o->resource);
	}

	free(private_server);
	private_loop->private_server = NULL;
	keep_private_server = NULL;
}
