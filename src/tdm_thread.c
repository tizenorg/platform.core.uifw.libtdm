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

#include <sys/socket.h>

#include "tdm.h"
#include "tdm_private.h"
#include "tdm_list.h"

struct _tdm_private_thread {
	tdm_private_loop *private_loop;

	pthread_t event_thread;

	pid_t display_tid;
	pid_t thread_tid;

	/* 0: read, 1: write */
	int pipe[2];
};

static void*
_tdm_thread_main(void *data)
{
	tdm_private_thread *private_thread = (tdm_private_thread*)data;
	tdm_private_loop *private_loop = private_thread->private_loop;
	int fd;
	struct pollfd fds;
	int ret;

	/* Not lock/unlock for the private_thread and private_loop structure
	 * because they won't be destroyed as long as tdm thread is running.
	 * When they're destroyed, we have already exit the tdm thread.
	 */
	private_thread->thread_tid = syscall(SYS_gettid);

	TDM_INFO("display_tid:%d, thread_tid: %d",
	         private_thread->display_tid, private_thread->thread_tid);

	fd = tdm_event_loop_get_fd(private_loop->dpy);
	if (fd < 0) {
		TDM_ERR("couldn't get fd");
		goto exit_thread;
	}

	fds.events = POLLIN;
	fds.fd = fd;
	fds.revents = 0;

	while (1) {
		if (tdm_debug_thread)
			TDM_INFO("server flush");
		tdm_event_loop_flush(private_loop->dpy);

		if (tdm_debug_thread)
			TDM_INFO("fd(%d) polling in", fd);

		ret = poll(&fds, 1, -1);

		if (tdm_debug_thread)
			TDM_INFO("fd(%d) polling out", fd);

		if (ret < 0) {
			if (errno == EBUSY)  /* normal case */
				continue;
			else {
				TDM_ERR("poll failed: %m");
				goto exit_thread;
			}
		}

		if (tdm_debug_thread)
			TDM_INFO("thread got events");

		if (tdm_event_loop_dispatch(private_loop->dpy) < 0)
			TDM_ERR("dispatch error");
	}

exit_thread:
	pthread_exit(NULL);
}

/* NOTE: tdm thread doesn't care about multi-thread. */
INTERN tdm_error
tdm_thread_init(tdm_private_loop *private_loop)
{
	tdm_private_display *private_display;
	tdm_private_thread *private_thread;
	const char *thread;

	TDM_RETURN_VAL_IF_FAIL(private_loop->dpy, TDM_ERROR_OPERATION_FAILED);

	private_display = private_loop->dpy;
	TDM_RETURN_VAL_IF_FAIL(private_display->private_loop, TDM_ERROR_OPERATION_FAILED);

	if (private_loop->private_thread)
		return TDM_ERROR_NONE;

	/* enable as default */
	thread = getenv("TDM_THREAD");
	if (!thread || !strncmp(thread, "1", 1)) {
		TDM_INFO("not using a TDM event thread");
		return TDM_ERROR_NONE;
	}

	private_thread = calloc(1, sizeof *private_thread);
	if (!private_thread) {
		TDM_ERR("alloc failed");
		return TDM_ERROR_OUT_OF_MEMORY;
	}

	if (pipe(private_thread->pipe) != 0) {
		TDM_ERR("socketpair failed: %m");
		free(private_thread);
		return TDM_ERROR_OPERATION_FAILED;
	}

	private_thread->private_loop = private_loop;
	private_loop->private_thread = private_thread;

	private_thread->display_tid = syscall(SYS_gettid);

	pthread_create(&private_thread->event_thread, NULL, _tdm_thread_main,
	               private_thread);

	TDM_INFO("using a TDM event thread. pipe(%d,%d)",
	         private_thread->pipe[0], private_thread->pipe[1]);

	return TDM_ERROR_NONE;
}

INTERN void
tdm_thread_deinit(tdm_private_loop *private_loop)
{
	if (!private_loop->private_thread)
		return;

	pthread_cancel(private_loop->private_thread->event_thread);
	pthread_join(private_loop->private_thread->event_thread, NULL);

	if (private_loop->private_thread->pipe[0] >= 0)
		close(private_loop->private_thread->pipe[0]);
	if (private_loop->private_thread->pipe[1] >= 0)
		close(private_loop->private_thread->pipe[1]);

	free(private_loop->private_thread);
	private_loop->private_thread = NULL;

	TDM_INFO("Finish a TDM event thread");
}

INTERN int
tdm_thread_get_fd(tdm_private_loop *private_loop)
{
	tdm_private_thread *private_thread;

	TDM_RETURN_VAL_IF_FAIL(private_loop, -1);
	TDM_RETURN_VAL_IF_FAIL(private_loop->private_thread, -1);

	private_thread = private_loop->private_thread;

	return private_thread->pipe[0];
}

INTERN tdm_error
tdm_thread_send_cb(tdm_private_loop *private_loop, tdm_thread_cb_base *base)
{
	tdm_private_thread *private_thread;
	ssize_t len;

	TDM_RETURN_VAL_IF_FAIL(base, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(private_loop, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(private_loop->private_thread, TDM_ERROR_INVALID_PARAMETER);

	private_thread = private_loop->private_thread;

	if (tdm_debug_thread)
		TDM_INFO("fd(%d) type(%d), length(%d)",
		         private_thread->pipe[1], base->type, base->length);

	len = write(private_thread->pipe[1], base, base->length);
	if (len != base->length) {
		TDM_ERR("write failed (%d != %d): %m", (int)len, base->length);
		return TDM_ERROR_OPERATION_FAILED;
	}

	return TDM_ERROR_NONE;
}

INTERN tdm_error
tdm_thread_handle_cb(tdm_private_loop *private_loop)
{
	tdm_private_thread *private_thread;
	tdm_thread_cb_base *base;
	char buffer[1024];
	int len, i;

	TDM_RETURN_VAL_IF_FAIL(private_loop, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(private_loop->private_thread, TDM_ERROR_INVALID_PARAMETER);

	private_thread = private_loop->private_thread;

	len = read(private_thread->pipe[0], buffer, sizeof buffer);

	if (tdm_debug_thread)
		TDM_INFO("fd(%d) read length(%d)", private_thread->pipe[0], len);

	if (len == 0)
		return TDM_ERROR_NONE;

	if (len < sizeof *base) {
		TDM_NEVER_GET_HERE();
		return TDM_ERROR_OPERATION_FAILED;
	}

	i = 0;
	while (i < len) {
		base = (tdm_thread_cb_base*)&buffer[i];
		if (tdm_debug_thread)
			TDM_INFO("type(%d), length(%d)", base->type, base->length);
		switch (base->type) {
		case TDM_THREAD_CB_OUTPUT_COMMIT:
		{
			tdm_thread_cb_output_commit *output_commit = (tdm_thread_cb_output_commit*)base;
			tdm_output *output_backend =
				tdm_display_find_output_stamp(private_loop->dpy, output_commit->output_stamp);
			if (!output_backend) {
				TDM_WRN("no output(%ld)", output_commit->output_stamp);
				break;
			}
			tdm_output_cb_commit(output_backend, output_commit->sequence,
			                     output_commit->tv_sec, output_commit->tv_usec,
			                     output_commit->user_data);
			break;
		}
		case TDM_THREAD_CB_OUTPUT_VBLANK:
		{
			tdm_thread_cb_output_vblank *output_vblank = (tdm_thread_cb_output_vblank*)base;
			tdm_output *output_backend =
				tdm_display_find_output_stamp(private_loop->dpy, output_vblank->output_stamp);
			if (!output_backend) {
				TDM_WRN("no output(%ld)", output_vblank->output_stamp);
				break;
			}
			tdm_output_cb_vblank(output_backend, output_vblank->sequence,
			                     output_vblank->tv_sec, output_vblank->tv_usec,
			                     output_vblank->user_data);
			break;
		}
		case TDM_THREAD_CB_OUTPUT_STATUS:
		{
			tdm_thread_cb_output_status *output_status = (tdm_thread_cb_output_status*)base;
			tdm_output *output_backend =
				tdm_display_find_output_stamp(private_loop->dpy, output_status->output_stamp);
			if (!output_backend) {
				TDM_WRN("no output(%ld)", output_status->output_stamp);
				break;
			}
			tdm_output_cb_status(output_backend, output_status->status,
			                     output_status->user_data);
			break;
		}
		case TDM_THREAD_CB_PP_DONE:
		{
			tdm_thread_cb_pp_done *pp_done = (tdm_thread_cb_pp_done*)base;
			tdm_pp *pp_backend =
				tdm_pp_find_stamp(private_loop->dpy, pp_done->pp_stamp);
			if (!pp_backend) {
				TDM_WRN("no pp(%ld)", pp_done->pp_stamp);
				break;
			}
			tdm_pp_cb_done(pp_backend, pp_done->src, pp_done->dst, pp_done->user_data);
			break;
		}
		case TDM_THREAD_CB_CAPTURE_DONE:
		{
			tdm_thread_cb_capture_done *capture_done = (tdm_thread_cb_capture_done*)base;
			tdm_capture *capture_backend =
				tdm_capture_find_stamp(private_loop->dpy, capture_done->capture_stamp);
			if (!capture_backend) {
				TDM_WRN("no capture(%ld)", capture_done->capture_stamp);
				break;
			}
			tdm_capture_cb_done(capture_backend, capture_done->buffer, capture_done->user_data);
			break;
		}
		default:
			break;
		}
		i += base->length;
	}

	tdm_event_loop_flush(private_loop->dpy);

	return TDM_ERROR_NONE;
}

INTERN int
tdm_thread_in_display_thread(tdm_private_loop *private_loop, pid_t tid)
{
	tdm_private_thread *private_thread;

	TDM_RETURN_VAL_IF_FAIL(private_loop, 1);

	if (!private_loop->private_thread)
		return 1;

	private_thread = private_loop->private_thread;

	return (private_thread->display_tid == tid) ? 1 : 0;
}
