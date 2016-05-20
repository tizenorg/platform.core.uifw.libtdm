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

#ifndef _TDM_PRIVATE_H_
#define _TDM_PRIVATE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <dirent.h>
#include <poll.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include <tbm_bufmgr.h>
#include <tbm_surface_queue.h>

#include "tdm_backend.h"
#include "tdm_log.h"
#include "tdm_list.h"
#include "tdm_macro.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define INIT_BUFMGR

/**
 * @file tdm_private.h
 * @brief The private header file for a frontend library
 */

extern int tdm_debug_buffer;
extern int tdm_debug_mutex;
extern int tdm_debug_thread;

#ifdef HAVE_TTRACE
#include <ttrace.h>
#define TDM_TRACE_BEGIN(NAME) traceBegin(TTRACE_TAG_GRAPHICS, "TDM:"#NAME)
#define TDM_TRACE_END() traceEnd(TTRACE_TAG_GRAPHICS)
#else
#define TDM_TRACE_BEGIN(NAME)
#define TDM_TRACE_END()
#endif

#define prototype_name_fn(res) const char * res##_str(int type)

prototype_name_fn(dpms);
prototype_name_fn(status);

typedef enum {
        TDM_CAPTURE_TARGET_OUTPUT,
        TDM_CAPTURE_TARGET_LAYER,
} tdm_capture_target;

typedef struct _tdm_private_display tdm_private_display;
typedef struct _tdm_private_output tdm_private_output;
typedef struct _tdm_private_layer tdm_private_layer;
typedef struct _tdm_private_pp tdm_private_pp;
typedef struct _tdm_private_capture tdm_private_capture;
typedef struct _tdm_private_loop tdm_private_loop;
typedef struct _tdm_private_server tdm_private_server;
typedef struct _tdm_private_thread tdm_private_thread;
typedef struct _tdm_private_vblank_handler tdm_private_vblank_handler;
typedef struct _tdm_private_commit_handler tdm_private_commit_handler;
typedef struct _tdm_private_change_handler tdm_private_change_handler;

struct _tdm_private_display {
	pthread_mutex_t lock;
	unsigned int init_count;

	/* backend module info */
	void *module;
	tdm_backend_module *module_data;
	tdm_backend_data *bdata;

#ifdef INIT_BUFMGR
	tbm_bufmgr bufmgr;
#endif

	/* backend function */
	tdm_display_capability capabilities;
	tdm_func_display func_display;
	tdm_func_output func_output;
	tdm_func_layer func_layer;
	tdm_func_pp func_pp;
	tdm_func_capture func_capture;

	/* backend capability */
	tdm_caps_display caps_display;
	tdm_caps_pp caps_pp;
	tdm_caps_capture caps_capture;

	/* output, pp list */
	struct list_head output_list;
	struct list_head pp_list;
	struct list_head capture_list;

	void **outputs_ptr;

	/* for event handling */
	tdm_private_loop *private_loop;
};

struct _tdm_private_output {
	struct list_head link;

	unsigned long stamp;

	tdm_private_display *private_display;

	tdm_caps_output caps;
	tdm_output *output_backend;

	unsigned int pipe;
	tdm_output_dpms current_dpms_value;

	int regist_vblank_cb;
	int regist_commit_cb;
	int regist_change_cb;

	struct list_head layer_list;
	struct list_head capture_list;
	struct list_head vblank_handler_list;
	struct list_head commit_handler_list;

	/* seperate list for multi-thread*/
	struct list_head change_handler_list_main;
	struct list_head change_handler_list_sub;

	void **layers_ptr;
};

struct _tdm_private_layer {
	struct list_head link;

	tdm_private_display *private_display;
	tdm_private_output *private_output;

	tdm_caps_layer caps;
	tdm_layer *layer_backend;

	tbm_surface_h pending_buffer;
	tbm_surface_h waiting_buffer;
	tbm_surface_h showing_buffer;
	tbm_surface_queue_h buffer_queue;

	struct list_head capture_list;

	unsigned int usable;
};

struct _tdm_private_pp {
	struct list_head link;

	unsigned long stamp;

	tdm_private_display *private_display;

	tdm_pp *pp_backend;

	struct list_head src_pending_buffer_list;
	struct list_head dst_pending_buffer_list;
	struct list_head src_buffer_list;
	struct list_head dst_buffer_list;

	pid_t owner_tid;
};

struct _tdm_private_capture {
	struct list_head link;
	struct list_head display_link;

	unsigned long stamp;

	tdm_capture_target target;

	tdm_private_display *private_display;
	tdm_private_output *private_output;
	tdm_private_layer *private_layer;

	tdm_capture *capture_backend;

	struct list_head pending_buffer_list;
	struct list_head buffer_list;

	pid_t owner_tid;
};

/* CAUTION:
 * Note that we don't need to (un)lock mutex to use this structure. If there is
 * no TDM thread, all TDM resources are protected by private_display's mutex.
 * If there is a TDM thread, this struct will be used only in a TDM thread.
 * So, we don't need to protect this structure by mutex. Not thread-safe.
 */
struct _tdm_private_loop {
	/* TDM uses wl_event_loop to handle various event sources including the TDM
	 * backend's fd.
	 */
	struct wl_display *wl_display;
	struct wl_event_loop *wl_loop;

	int backend_fd;
	tdm_event_loop_source *backend_source;

	/* In event loop, all resources are accessed by this dpy.
	 * CAUTION:
	 * - DO NOT include other private structure in this structure because this
	 *   struct is not protected by mutex.
	 */
	tdm_display *dpy;

	/* for handling TDM client requests */
	tdm_private_server *private_server;

	/* To have a TDM event thread. If TDM_THREAD enviroment variable is not set
	 * private_thread is NULL.
	 */
	tdm_private_thread *private_thread;
};

struct _tdm_private_vblank_handler {
	struct list_head link;

	tdm_private_output *private_output;
	tdm_output_vblank_handler func;
	void *user_data;

	pid_t owner_tid;
};

struct _tdm_private_commit_handler {
	struct list_head link;

	tdm_private_output *private_output;
	tdm_output_commit_handler func;
	void *user_data;

	pid_t owner_tid;
};

struct _tdm_private_change_handler {
	struct list_head link;

	tdm_private_output *private_output;
	tdm_output_change_handler func;
	void *user_data;

	pid_t owner_tid;
};

typedef struct _tdm_buffer_info {
	tbm_surface_h buffer;

	/* ref_count for backend */
	int backend_ref_count;

	struct list_head release_funcs;
	struct list_head destroy_funcs;

	struct list_head *list;
	struct list_head link;
} tdm_buffer_info;

const char*
tdm_get_dpms_str(tdm_output_dpms dpms_value);

int
tdm_display_check_module_abi(tdm_private_display *private_display, int abimaj, int abimin);

tdm_private_output *
tdm_display_find_output_stamp(tdm_private_display *private_display,
                              unsigned long stamp);
tdm_private_pp *
tdm_pp_find_stamp(tdm_private_display *private_display, unsigned long stamp);
tdm_private_capture *
tdm_capture_find_stamp(tdm_private_display *private_display, unsigned long stamp);

void
tdm_output_cb_vblank(tdm_output *output_backend, unsigned int sequence,
                     unsigned int tv_sec, unsigned int tv_usec, void *user_data);
void
tdm_output_cb_commit(tdm_output *output_backend, unsigned int sequence,
                     unsigned int tv_sec, unsigned int tv_usec, void *user_data);
void
tdm_output_cb_status(tdm_output *output_backend, tdm_output_conn_status status,
                     void *user_data);
void
tdm_pp_cb_done(tdm_pp *pp_backend, tbm_surface_h src, tbm_surface_h dst,
               void *user_data);
void
tdm_capture_cb_done(tdm_capture *capture_backend, tbm_surface_h buffer,
                    void *user_data);

void
tdm_output_call_change_handler_internal(tdm_private_output *private_output,
                                        struct list_head *change_handler_list,
                                        tdm_output_change_type type,
                                        tdm_value value);

tdm_private_pp *
tdm_pp_create_internal(tdm_private_display *private_display, tdm_error *error);
void
tdm_pp_destroy_internal(tdm_private_pp *private_pp);

tdm_private_capture *
tdm_capture_create_output_internal(tdm_private_output *private_output,
                                   tdm_error *error);
tdm_private_capture *
tdm_capture_create_layer_internal(tdm_private_layer *private_layer,
                                  tdm_error *error);
void
tdm_capture_destroy_internal(tdm_private_capture *private_capture);

/* utility buffer functions for private */
tdm_buffer_info*
tdm_buffer_get_info(tbm_surface_h buffer);
tbm_surface_h
tdm_buffer_list_get_first_entry(struct list_head *list);
void
tdm_buffer_list_dump(struct list_head *list);

/* event functions for private */
tdm_error
tdm_event_loop_init(tdm_private_display *private_display);
void
tdm_event_loop_deinit(tdm_private_display *private_display);
void
tdm_event_loop_create_backend_source(tdm_private_display *private_display);
int
tdm_event_loop_get_fd(tdm_private_display *private_display);
tdm_error
tdm_event_loop_dispatch(tdm_private_display *private_display);
void
tdm_event_loop_flush(tdm_private_display *private_display);

typedef enum {
	TDM_THREAD_CB_NONE,
	TDM_THREAD_CB_OUTPUT_COMMIT,
	TDM_THREAD_CB_OUTPUT_VBLANK,
	TDM_THREAD_CB_OUTPUT_STATUS,
	TDM_THREAD_CB_PP_DONE,
	TDM_THREAD_CB_CAPTURE_DONE,
} tdm_thread_cb_type;

typedef struct _tdm_thread_cb_base tdm_thread_cb_base;
typedef struct _tdm_thread_cb_output_vblank tdm_thread_cb_output_commit;
typedef struct _tdm_thread_cb_output_vblank tdm_thread_cb_output_vblank;
typedef struct _tdm_thread_cb_output_status tdm_thread_cb_output_status;
typedef struct _tdm_thread_cb_pp_done tdm_thread_cb_pp_done;
typedef struct _tdm_thread_cb_capture_done tdm_thread_cb_capture_done;

struct _tdm_thread_cb_base {
	tdm_thread_cb_type type;
	unsigned int length;
};

struct _tdm_thread_cb_output_vblank {
	tdm_thread_cb_base base;
	unsigned long output_stamp;
	unsigned int sequence;
	unsigned int tv_sec;
	unsigned int tv_usec;
	void *user_data;
};

struct _tdm_thread_cb_output_status {
	tdm_thread_cb_base base;
	unsigned long output_stamp;
	tdm_output_conn_status status;
	void *user_data;
};

struct _tdm_thread_cb_pp_done {
	tdm_thread_cb_base base;
	unsigned long pp_stamp;
	tbm_surface_h src;
	tbm_surface_h dst;
	void *user_data;
};

struct _tdm_thread_cb_capture_done {
	tdm_thread_cb_base base;
	unsigned long capture_stamp;
	tbm_surface_h buffer;
	void *user_data;
};

tdm_error
tdm_thread_init(tdm_private_loop *private_loop);
void
tdm_thread_deinit(tdm_private_loop *private_loop);
int
tdm_thread_get_fd(tdm_private_loop *private_loop);
tdm_error
tdm_thread_send_cb(tdm_private_loop *private_loop, tdm_thread_cb_base *base);
tdm_error
tdm_thread_handle_cb(tdm_private_loop *private_loop);
int
tdm_thread_in_display_thread(pid_t tid);
int
tdm_thread_is_running(void);

tdm_error
tdm_server_init(tdm_private_loop *private_loop);
void
tdm_server_deinit(tdm_private_loop *private_loop);

unsigned long
tdm_helper_get_time_in_millis(void);
unsigned long
tdm_helper_get_time_in_micros(void);

extern pthread_mutex_t tdm_mutex_check_lock;
extern int tdm_mutex_locked;
extern int tdm_dump_enable;

#define _pthread_mutex_unlock(l) \
	do { \
		if (tdm_debug_mutex) \
			TDM_INFO("mutex unlock"); \
		pthread_mutex_lock(&tdm_mutex_check_lock); \
		tdm_mutex_locked = 0; \
		pthread_mutex_unlock(&tdm_mutex_check_lock); \
		pthread_mutex_unlock(l); \
	} while (0)
#ifdef TDM_CONFIG_MUTEX_TIMEOUT
#define MUTEX_TIMEOUT_SEC 5
#define _pthread_mutex_lock(l) \
	do { \
		if (tdm_debug_mutex) \
			TDM_INFO("mutex lock"); \
		struct timespec rtime; \
		clock_gettime(CLOCK_REALTIME, &rtime); \
		rtime.tv_sec += MUTEX_TIMEOUT_SEC; \
		if (pthread_mutex_timedlock(l, &rtime)) { \
			TDM_ERR("Mutex lock failed PID %d", getpid()); \
			_pthread_mutex_unlock(l); \
		} \
		else { \
			pthread_mutex_lock(&tdm_mutex_check_lock); \
			tdm_mutex_locked = 1; \
			pthread_mutex_unlock(&tdm_mutex_check_lock); \
		} \
	} while (0)
#else //TDM_CONFIG_MUTEX_TIMEOUT
#define _pthread_mutex_lock(l) \
	do { \
		if (tdm_debug_mutex) \
			TDM_INFO("mutex lock"); \
		pthread_mutex_lock(l); \
		pthread_mutex_lock(&tdm_mutex_check_lock); \
		tdm_mutex_locked = 1; \
		pthread_mutex_unlock(&tdm_mutex_check_lock); \
	} while (0)
#endif //TDM_CONFIG_MUTEX_TIMEOUT
//#define TDM_MUTEX_IS_LOCKED() (tdm_mutex_locked == 1)
static inline int TDM_MUTEX_IS_LOCKED(void)
{
	int ret;
	pthread_mutex_lock(&tdm_mutex_check_lock);
	ret = (tdm_mutex_locked == 1);
	pthread_mutex_unlock(&tdm_mutex_check_lock);
	return ret;
}

tdm_error
_tdm_display_lock(tdm_display *dpy, const char *func);
void
_tdm_display_unlock(tdm_display *dpy, const char *func);

#define tdm_display_lock(dpy)   _tdm_display_lock(dpy, __FUNCTION__)
#define tdm_display_unlock(dpy)   _tdm_display_unlock(dpy, __FUNCTION__)

tdm_error
tdm_display_update_output(tdm_private_display *private_display,
							tdm_output *output_backend, int pipe);

#ifdef __cplusplus
}
#endif

#endif /* _TDM_PRIVATE_H_ */
