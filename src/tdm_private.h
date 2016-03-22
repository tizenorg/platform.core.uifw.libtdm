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

#include <tbm_bufmgr.h>
#include <tbm_surface_queue.h>

#include "tdm_backend.h"
#include "tdm_log.h"
#include "tdm_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file tdm_private.h
 * @brief The private header file for a frontend library
 */

#undef EXTERN
#undef DEPRECATED
#undef INTERN

#if defined(__GNUC__) && __GNUC__ >= 4
#define EXTERN __attribute__ ((visibility("default")))
#else
#define EXTERN
#endif

#if defined(__GNUC__) && __GNUC__ >= 4
#define INTERN __attribute__ ((visibility("hidden")))
#else
#define INTERN
#endif

#if defined(__GNUC__) && __GNUC__ >= 4
#define DEPRECATED __attribute__ ((deprecated))
#else
#define DEPRECATED
#endif

/* check condition */
#define TDM_RETURN_IF_FAIL(cond) {\
    if (!(cond)) {\
        TDM_ERR ("'%s' failed", #cond);\
        return;\
    }\
}
#define TDM_RETURN_VAL_IF_FAIL(cond, val) {\
    if (!(cond)) {\
        TDM_ERR ("'%s' failed", #cond);\
        return val;\
    }\
}
#define TDM_RETURN_VAL_IF_FAIL_WITH_ERROR(cond, error_v, val) {\
    if (!(cond)) {\
        TDM_ERR ("'%s' failed", #cond);\
        ret = error_v;\
        if (error) *error = ret;\
        return val;\
    }\
}

#define TDM_WARNING_IF_FAIL(cond)  {\
    if (!(cond))\
        TDM_ERR ("'%s' failed", #cond);\
}
#define TDM_GOTO_IF_FAIL(cond, dst) {\
    if (!(cond)) {\
        TDM_ERR ("'%s' failed", #cond);\
        goto dst;\
    }\
}

#ifdef HAVE_TTRACE
#include <ttrace.h>
#define TDM_TRACE_BEGIN(NAME) traceBegin(TTRACE_TAG_GRAPHICS, "TDM:"#NAME)
#define TDM_TRACE_END() traceEnd(TTRACE_TAG_GRAPHICS)
#else
#define TDM_TRACE_BEGIN(NAME)
#define TDM_TRACE_END()
#endif

#define TDM_NEVER_GET_HERE() TDM_ERR("** NEVER GET HERE **")

#define TDM_SNPRINTF(p, len, fmt, ARG...)  \
    do { \
        if (p && len && *len > 0) \
        { \
            int s = snprintf(p, *len, fmt, ##ARG); \
            p += s; \
            *len -= s; \
        } \
    } while (0)

#define C(b,m)              (((b) >> (m)) & 0xFF)
#define B(c,s)              ((((unsigned int)(c)) & 0xff) << (s))
#define FOURCC(a,b,c,d)     (B(d,24) | B(c,16) | B(b,8) | B(a,0))
#define FOURCC_STR(id)      C(id,0), C(id,8), C(id,16), C(id,24)
#define FOURCC_ID(str)      FOURCC(((char*)str)[0],((char*)str)[1],((char*)str)[2],((char*)str)[3])

typedef enum {
        TDM_CAPTURE_TARGET_OUTPUT,
        TDM_CAPTURE_TARGET_LAYER,
} tdm_capture_target;

typedef struct _tdm_private_display tdm_private_display;
typedef struct _tdm_private_output tdm_private_output;
typedef struct _tdm_private_layer tdm_private_layer;
typedef struct _tdm_private_pp tdm_private_pp;
typedef struct _tdm_private_capture tdm_private_capture;
typedef struct _tdm_private_vblank_handler tdm_private_vblank_handler;
typedef struct _tdm_private_commit_handler tdm_private_commit_handler;

struct _tdm_private_display {
	pthread_mutex_t lock;
	unsigned int init_count;

	/* backend module info */
	void *module;
	tdm_backend_module *module_data;
	tdm_backend_data *bdata;

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

	void **outputs_ptr;
};

struct _tdm_private_output {
	struct list_head link;

	tdm_private_display *private_display;

	tdm_caps_output caps;
	tdm_output *output_backend;

	unsigned int pipe;

	int regist_vblank_cb;
	int regist_commit_cb;

	struct list_head layer_list;
	struct list_head capture_list;
	struct list_head vblank_handler_list;
	struct list_head commit_handler_list;

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

	tdm_private_display *private_display;

	tdm_pp *pp_backend;

	struct list_head src_pending_buffer_list;
	struct list_head dst_pending_buffer_list;
	struct list_head src_buffer_list;
	struct list_head dst_buffer_list;
};

struct _tdm_private_capture {
	struct list_head link;

	tdm_capture_target target;

	tdm_private_display *private_display;
	tdm_private_output *private_output;
	tdm_private_layer *private_layer;

	tdm_capture *capture_backend;

	struct list_head pending_buffer_list;
	struct list_head buffer_list;
};

struct _tdm_private_vblank_handler {
	struct list_head link;

	tdm_private_output *private_output;
	tdm_output_vblank_handler func;
	void *user_data;
};

struct _tdm_private_commit_handler {
	struct list_head link;

	tdm_private_output *private_output;
	tdm_output_commit_handler func;
	void *user_data;
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

#ifdef __cplusplus
}
#endif

#endif /* _TDM_PRIVATE_H_ */
