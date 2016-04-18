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

#include <wayland-server-core.h>

typedef struct _tdm_event_loop_source_base
{
	struct wl_event_source *wl_source;
} tdm_event_loop_source_base;

typedef struct _tdm_event_loop_source_fd
{
	tdm_event_loop_source_base base;
	tdm_private_display *private_display;
	tdm_event_loop_fd_handler func;
	void *user_data;
} tdm_event_loop_source_fd;

typedef struct _tdm_event_loop_source_timer
{
	tdm_event_loop_source_base base;
	tdm_private_display *private_display;
	tdm_event_loop_timer_handler func;
	void *user_data;
} tdm_event_loop_source_timer;

static tdm_error
_tdm_event_loop_main_fd_handler(int fd, tdm_event_loop_mask mask, void *user_data)
{
	tdm_private_display *private_display = (tdm_private_display*)user_data;
	tdm_private_loop *private_loop;
	tdm_func_display *func_display;
	tdm_error ret;

	TDM_RETURN_VAL_IF_FAIL(private_display != NULL, TDM_ERROR_OPERATION_FAILED);
	TDM_RETURN_VAL_IF_FAIL(private_display->private_loop != NULL, TDM_ERROR_OPERATION_FAILED);

	private_loop = private_display->private_loop;

	if (tdm_debug_thread)
		TDM_INFO("backend fd(%d) event happens", private_loop->backend_fd);

	func_display = &private_display->func_display;
	if (!func_display->display_handle_events)
		return TDM_ERROR_NONE;

	ret = func_display->display_handle_events(private_display->bdata);

	return ret;
}

INTERN tdm_error
tdm_event_loop_init(tdm_private_display *private_display)
{
	tdm_private_loop *private_loop;
	tdm_error ret;

	if (private_display->private_loop)
		return TDM_ERROR_NONE;

	private_loop = calloc(1, sizeof *private_loop);
	if (!private_loop) {
		TDM_ERR("alloc failed");
		return TDM_ERROR_OUT_OF_MEMORY;
	}

	private_loop->backend_fd = -1;

	private_loop->wl_display = wl_display_create();
	if (!private_loop->wl_display) {
		TDM_ERR("creating a wayland display failed");
		free(private_loop);
		return TDM_ERROR_OUT_OF_MEMORY;
	}
	private_loop->wl_loop = wl_display_get_event_loop(private_loop->wl_display);

	ret = tdm_server_init(private_loop);
	if (ret != TDM_ERROR_NONE) {
		TDM_ERR("server init failed");
		wl_display_destroy(private_loop->wl_display);
		free(private_loop);
		return TDM_ERROR_OPERATION_FAILED;
	}

	private_loop->dpy = private_display;
	private_display->private_loop = private_loop;

	ret = tdm_thread_init(private_loop);
	if (ret != TDM_ERROR_NONE) {
		TDM_ERR("thread init failed");
		tdm_server_deinit(private_loop);
		wl_display_destroy(private_loop->wl_display);
		free(private_loop);
		return TDM_ERROR_OPERATION_FAILED;
	}

	TDM_INFO("event loop fd(%d)", wl_event_loop_get_fd(private_loop->wl_loop));

	return TDM_ERROR_NONE;
}

INTERN void
tdm_event_loop_deinit(tdm_private_display *private_display)
{
	if (!private_display->private_loop)
		return;

	tdm_thread_deinit(private_display->private_loop);
	tdm_server_deinit(private_display->private_loop);

	if (private_display->private_loop->backend_source)
		tdm_event_loop_source_remove(private_display->private_loop->backend_source);

	if (private_display->private_loop->wl_display)
		wl_display_destroy(private_display->private_loop->wl_display);

	free(private_display->private_loop);
	private_display->private_loop = NULL;
}

INTERN void
tdm_event_loop_create_backend_source(tdm_private_display *private_display)
{
	tdm_private_loop *private_loop = private_display->private_loop;
	tdm_func_display *func_display;
	tdm_error ret;
	int fd = -1;

	TDM_RETURN_IF_FAIL(private_loop != NULL);

	func_display = &private_display->func_display;
	if (!func_display->display_get_fd) {
		TDM_INFO("TDM backend module won't offer a display fd");
		return;
	}

	ret = func_display->display_get_fd(private_display->bdata, &fd);
	if (fd < 0) {
		TDM_WRN("TDM backend module returns fd(%d)", fd);
		return;
	}

	if (!func_display->display_handle_events) {
		TDM_ERR("no display_handle_events function");
		return;
	}

	private_loop->backend_source =
		tdm_event_loop_add_fd_handler(private_display, fd,
		                              TDM_EVENT_LOOP_READABLE,
		                              _tdm_event_loop_main_fd_handler,
		                              private_display, &ret);
	if (!private_loop->backend_source) {
		TDM_ERR("no backend fd(%d) source", fd);
		return;
	}

	private_loop->backend_fd = fd;

	TDM_INFO("backend fd(%d) source created", private_loop->backend_fd);
}

INTERN int
tdm_event_loop_get_fd(tdm_private_display *private_display)
{
	tdm_private_loop *private_loop = private_display->private_loop;

	TDM_RETURN_VAL_IF_FAIL(private_loop->wl_loop != NULL, -1);

	return wl_event_loop_get_fd(private_loop->wl_loop);
}

INTERN tdm_error
tdm_event_loop_dispatch(tdm_private_display *private_display)
{
	tdm_private_loop *private_loop = private_display->private_loop;

	TDM_RETURN_VAL_IF_FAIL(private_loop->wl_loop != NULL, TDM_ERROR_OPERATION_FAILED);

	if (tdm_debug_thread)
		TDM_INFO("dispatch");

	/* Don't set timeout to -1. It can make deadblock by two mutex locks.
	 * If need to set -1, use poll() and call tdm_event_loop_dispatch() after
	 * escaping polling.
	 */
	if (wl_event_loop_dispatch(private_loop->wl_loop, 0) < 0)
		TDM_ERR("dispatch failed");

	return TDM_ERROR_NONE;
}


INTERN void
tdm_event_loop_flush(tdm_private_display *private_display)
{
	tdm_private_loop *private_loop = private_display->private_loop;

	TDM_RETURN_IF_FAIL(private_loop->wl_display != NULL);

	wl_display_flush_clients(private_loop->wl_display);
}

static int
_tdm_event_loop_fd_func(int fd, uint32_t wl_mask, void *data)
{
	tdm_event_loop_source_fd *fd_source = (tdm_event_loop_source_fd*)data;
	tdm_event_loop_mask mask = 0;

	TDM_RETURN_VAL_IF_FAIL(fd_source, 1);
	TDM_RETURN_VAL_IF_FAIL(fd_source->func, 1);

	if (wl_mask & WL_EVENT_READABLE)
		mask |= TDM_EVENT_LOOP_READABLE;
	if (wl_mask & WL_EVENT_WRITABLE)
		mask |= TDM_EVENT_LOOP_WRITABLE;
	if (wl_mask & WL_EVENT_HANGUP)
		mask |= TDM_EVENT_LOOP_HANGUP;
	if (wl_mask & WL_EVENT_ERROR)
		mask |= TDM_EVENT_LOOP_ERROR;

	fd_source->func(fd, mask, fd_source->user_data);

	return 1;
}

EXTERN tdm_event_loop_source*
tdm_event_loop_add_fd_handler(tdm_display *dpy, int fd, tdm_event_loop_mask mask,
                              tdm_event_loop_fd_handler func, void *user_data,
                              tdm_error *error)
{
	tdm_private_display *private_display;
	tdm_private_loop *private_loop;
	tdm_event_loop_source_fd *fd_source;
	uint32_t wl_mask = 0;
	tdm_error ret;

	TDM_RETURN_VAL_IF_FAIL_WITH_ERROR(dpy, TDM_ERROR_INVALID_PARAMETER, NULL);
	TDM_RETURN_VAL_IF_FAIL_WITH_ERROR(fd >= 0, TDM_ERROR_INVALID_PARAMETER, NULL);
	TDM_RETURN_VAL_IF_FAIL_WITH_ERROR(func, TDM_ERROR_INVALID_PARAMETER, NULL);

	private_display = (tdm_private_display*)dpy;
	private_loop = private_display->private_loop;
	TDM_RETURN_VAL_IF_FAIL_WITH_ERROR(private_loop, TDM_ERROR_INVALID_PARAMETER, NULL);
	TDM_RETURN_VAL_IF_FAIL_WITH_ERROR(private_loop->wl_loop, TDM_ERROR_INVALID_PARAMETER, NULL);

	fd_source = calloc(1, sizeof(tdm_event_loop_source_fd));
	TDM_RETURN_VAL_IF_FAIL_WITH_ERROR(fd_source, TDM_ERROR_OUT_OF_MEMORY, NULL);

	if (mask & TDM_EVENT_LOOP_READABLE)
		wl_mask |= WL_EVENT_READABLE;
	if (mask & TDM_EVENT_LOOP_WRITABLE)
		wl_mask |= WL_EVENT_WRITABLE;

	fd_source->base.wl_source =
		wl_event_loop_add_fd(private_loop->wl_loop,
		                     fd, wl_mask, _tdm_event_loop_fd_func, fd_source);
	if (!fd_source->base.wl_source) {
		if (error)
			*error = TDM_ERROR_OUT_OF_MEMORY;
		free(fd_source);
		return NULL;
	}

	fd_source->private_display = private_display;
	fd_source->func = func;
	fd_source->user_data = user_data;

	if (error)
		*error = TDM_ERROR_NONE;

	return (tdm_event_loop_source*)fd_source;
}

EXTERN tdm_error
tdm_event_loop_source_fd_update(tdm_event_loop_source *source, tdm_event_loop_mask mask)
{
	tdm_event_loop_source_fd *fd_source = source;
	uint32_t wl_mask = 0;

	TDM_RETURN_VAL_IF_FAIL(fd_source, TDM_ERROR_INVALID_PARAMETER);

	if (mask & TDM_EVENT_LOOP_READABLE)
		wl_mask |= WL_EVENT_READABLE;
	if (mask & TDM_EVENT_LOOP_WRITABLE)
		wl_mask |= WL_EVENT_WRITABLE;

	if (wl_event_source_fd_update(fd_source->base.wl_source, wl_mask) < 0) {
		TDM_ERR("source update failed: %m");
		return TDM_ERROR_OPERATION_FAILED;
	}

	return TDM_ERROR_NONE;
}

static int
_tdm_event_loop_timer_func(void *data)
{
	tdm_event_loop_source_timer *timer_source = (tdm_event_loop_source_timer*)data;

	TDM_RETURN_VAL_IF_FAIL(timer_source, 1);
	TDM_RETURN_VAL_IF_FAIL(timer_source->func, 1);

	timer_source->func(timer_source->user_data);

	return 1;
}

EXTERN tdm_event_loop_source*
tdm_event_loop_add_timer_handler(tdm_display *dpy, tdm_event_loop_timer_handler func,
                                 void *user_data, tdm_error *error)
{
	tdm_private_display *private_display;
	tdm_private_loop *private_loop;
	tdm_event_loop_source_timer *timer_source;
	tdm_error ret;

	TDM_RETURN_VAL_IF_FAIL_WITH_ERROR(dpy, TDM_ERROR_INVALID_PARAMETER, NULL);
	TDM_RETURN_VAL_IF_FAIL_WITH_ERROR(func, TDM_ERROR_INVALID_PARAMETER, NULL);

	private_display = (tdm_private_display*)dpy;
	private_loop = private_display->private_loop;
	TDM_RETURN_VAL_IF_FAIL_WITH_ERROR(private_loop, TDM_ERROR_INVALID_PARAMETER, NULL);
	TDM_RETURN_VAL_IF_FAIL_WITH_ERROR(private_loop->wl_loop, TDM_ERROR_INVALID_PARAMETER, NULL);

	timer_source = calloc(1, sizeof(tdm_event_loop_source_timer));
	TDM_RETURN_VAL_IF_FAIL_WITH_ERROR(timer_source, TDM_ERROR_OUT_OF_MEMORY, NULL);

	timer_source->base.wl_source =
		wl_event_loop_add_timer(private_loop->wl_loop,
		                        _tdm_event_loop_timer_func, timer_source);
	if (!timer_source->base.wl_source) {
		if (error)
			*error = TDM_ERROR_OUT_OF_MEMORY;
		free(timer_source);
		return NULL;
	}

	timer_source->private_display = private_display;
	timer_source->func = func;
	timer_source->user_data = user_data;

	if (error)
		*error = TDM_ERROR_NONE;

	return (tdm_event_loop_source*)timer_source;
}

EXTERN tdm_error
tdm_event_loop_source_timer_update(tdm_event_loop_source *source, int ms_delay)
{
	tdm_event_loop_source_timer *timer_source = source;

	TDM_RETURN_VAL_IF_FAIL(timer_source, TDM_ERROR_INVALID_PARAMETER);

	if (wl_event_source_timer_update(timer_source->base.wl_source, ms_delay) < 0) {
		TDM_ERR("source update failed: %m");
		return TDM_ERROR_OPERATION_FAILED;
	}

	return TDM_ERROR_NONE;
}

EXTERN void
tdm_event_loop_source_remove(tdm_event_loop_source *source)
{
	tdm_event_loop_source_base *base = (tdm_event_loop_source_base*)source;

	if (!base)
		return;

	wl_event_source_remove(base->wl_source);

	free(source);
}
