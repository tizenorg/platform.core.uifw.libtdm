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
 */

struct _tdm_private_server {
	struct list_head vblank_list;
};

typedef struct _tdm_server_vblank_info {
	struct list_head link;
	struct wl_resource *resource;
	tdm_private_server *private_server;
} tdm_server_vblank_info;

static tdm_private_server *keep_private_server;
static int tdm_debug_server;

static void
_tdm_server_cb_output_vblank(tdm_output *output, unsigned int sequence,
                             unsigned int tv_sec, unsigned int tv_usec,
                             void *user_data)
{
	tdm_server_vblank_info *vblank_info = (tdm_server_vblank_info*)user_data;
	tdm_server_vblank_info *found;

	if (!keep_private_server)
		return;

	LIST_FIND_ITEM(vblank_info, &keep_private_server->vblank_list,
	               tdm_server_vblank_info, link, found);
	if (!found) {
		TDM_DBG("vblank_info(%p) is destroyed", vblank_info);
		return;
	}

	if (tdm_debug_server) {
		unsigned long curr = tdm_helper_get_time_in_micros();
		unsigned long vtime = (tv_sec * 1000000) + tv_usec;
		if (curr - vtime > 1000) /* 1ms */
			TDM_WRN("delay: %d us", (int)(curr - vtime));
	}

	TDM_DBG("wl_tdm_vblank@%d done", wl_resource_get_id(vblank_info->resource));

	wl_tdm_vblank_send_done(vblank_info->resource, sequence, tv_sec, tv_usec);
	wl_resource_destroy(vblank_info->resource);
}

static void
destroy_vblank_callback(struct wl_resource *resource)
{
	tdm_server_vblank_info *vblank_info = wl_resource_get_user_data(resource);
	LIST_DEL(&vblank_info->link);
	free(vblank_info);
}

static void
_tdm_server_cb_wait_vblank(struct wl_client *client,
                           struct wl_resource *resource,
                           uint32_t id, const char *name, int32_t interval)
{
	tdm_private_loop *private_loop = wl_resource_get_user_data(resource);
	tdm_private_server *private_server = private_loop->private_server;
	tdm_server_vblank_info *vblank_info;
	struct wl_resource *vblank_resource;
	tdm_output *found = NULL;
	int count = 0, i;
	tdm_error ret;

	TDM_DBG("The tdm client requests vblank");

	tdm_display_get_output_count(private_loop->dpy, &count);

	for (i = 0; i < count; i++) {
		tdm_output *output= tdm_display_get_output(private_loop->dpy, i, NULL);
		tdm_output_conn_status status;
		const char *model = NULL;

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

	if (!found) {
		wl_resource_post_error(resource, WL_TDM_ERROR_INVALID_NAME,
		                       "There is no '%s' output", name);
		TDM_ERR("There is no '%s' output", name);
		return;
	}

	vblank_resource =
		wl_resource_create(client, &wl_tdm_vblank_interface,
		                   wl_resource_get_version(resource), id);
	if (!vblank_resource) {
		wl_resource_post_no_memory(resource);
		TDM_ERR("wl_resource_create failed");
		return;
	}

	vblank_info = calloc(1, sizeof *vblank_info);
	if (!vblank_info) {
		wl_resource_destroy(vblank_resource);
		wl_resource_post_no_memory(resource);
		TDM_ERR("alloc failed");
		return;
	}

	TDM_DBG("wl_tdm_vblank@%d output(%s) interval(%d)", id, name, interval);

	ret = tdm_output_wait_vblank(found, interval, 0,
	                             _tdm_server_cb_output_vblank, vblank_info);
	if (ret != TDM_ERROR_NONE) {
		wl_resource_destroy(vblank_resource);
		free(vblank_info);
		wl_resource_post_error(resource, WL_TDM_ERROR_OPERATION_FAILED,
		                       "couldn't wait vblank for %s", name);
		TDM_ERR("couldn't wait vblank for %s", name);
		return;
	}

	vblank_info->resource = vblank_resource;
	vblank_info->private_server = private_server;

	wl_resource_set_implementation(vblank_resource, NULL, vblank_info,
	                               destroy_vblank_callback);

	LIST_ADDTAIL(&vblank_info->link, &private_server->vblank_list);
}

static const struct wl_tdm_interface tdm_implementation = {
	_tdm_server_cb_wait_vblank,
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

	LIST_INITHEAD(&private_server->vblank_list);

	if (!wl_global_create(private_loop->wl_display, &wl_tdm_interface, 1,
	                      private_loop, _tdm_server_bind)) {
		TDM_ERR("creating a global resource failed");
		free(private_server);
		return TDM_ERROR_OUT_OF_MEMORY;
	}

	private_loop->private_server = private_server;
	keep_private_server = private_server;

	return TDM_ERROR_NONE;
}

INTERN void
tdm_server_deinit(tdm_private_loop *private_loop)
{
	tdm_server_vblank_info *v = NULL, *vv = NULL;
	tdm_private_server *private_server;

	if (!private_loop->private_server)
		return;

	private_server = private_loop->private_server;

	LIST_FOR_EACH_ENTRY_SAFE(v, vv, &private_server->vblank_list, link) {
		wl_resource_destroy(v->resource);
	}

	free(private_server);
	private_loop->private_server = NULL;
	keep_private_server = NULL;
}
