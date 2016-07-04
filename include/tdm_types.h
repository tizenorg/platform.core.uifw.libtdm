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

#ifndef _TDM_TYPES_H_
#define _TDM_TYPES_H_

#include <tbm_surface.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file tdm_types.h
 * @brief The header file which defines Enumerations and Structures for frontend and backend.
 * @details
 * Both frontend(@ref tdm.h) and backend(@ref tdm_backend.h) header files
 * include @ref tdm_types.h
 * @par Example
 * @code
 * #include <tdm.h>    //for a frontend user
 * @endcode
 * @code
 * #include <tdm_backend.h>  //for a vendor to implement a backend module
 * @endcode
 */

#include <tdm_common.h>

typedef enum {
	TDM_EVENT_LOOP_READABLE = (1 << 0),
	TDM_EVENT_LOOP_WRITABLE = (1 << 1),
	TDM_EVENT_LOOP_HANGUP   = (1 << 2),
	TDM_EVENT_LOOP_ERROR    = (1 << 3),
} tdm_event_loop_mask;

/**
 * @brief The output mode structure
 */
typedef struct _tdm_output_mode {
	unsigned int clock;
	unsigned int hdisplay, hsync_start, hsync_end, htotal, hskew;
	unsigned int vdisplay, vsync_start, vsync_end, vtotal, vscan;
	unsigned int vrefresh;
	unsigned int flags;
	unsigned int type;
	char name[TDM_NAME_LEN];
} tdm_output_mode;

/**
 * @brief The property structure
 */
typedef struct _tdm_prop {
	unsigned int id;
	char name[TDM_NAME_LEN];
} tdm_prop;

/**
 * @brief The info config structure
 */
typedef struct _tdm_info_config {
	tdm_size size;
	tdm_pos pos;
	tbm_format format;
} tdm_info_config;

/**
 * @brief The layer info structre
 */
typedef struct _tdm_info_layer {
	tdm_info_config src_config;
	tdm_pos dst_pos;
	tdm_transform transform;
} tdm_info_layer;

/**
 * @brief The pp info structre
 */
typedef struct _tdm_info_pp {
	tdm_info_config src_config;
	tdm_info_config dst_config;
	tdm_transform transform;
	int sync;
	int flags;
} tdm_info_pp;

/**
 * @brief The capture info structre
 */
typedef struct _tdm_info_capture {
	tdm_info_config dst_config;
	tdm_transform transform;
	int oneshot;
	int frequency;
	int flags;
} tdm_info_capture;

/**
 * @brief The tdm display object
 */
typedef void tdm_display;

/**
 * @brief The tdm output object
 */
typedef void tdm_output;

/**
 * @brief The tdm layer object
 */
typedef void tdm_layer;

/**
 * @brief The tdm capture object
 */
typedef void tdm_capture;

/**
 * @brief The tdm pp object
 */
typedef void tdm_pp;

/**
 * @brief The vblank handler
 * @see output_set_vblank_handler() function of #tdm_func_display
 */
typedef void (*tdm_output_vblank_handler)(tdm_output *output, unsigned int sequence,
										  unsigned int tv_sec, unsigned int tv_usec,
										  void *user_data);

/**
 * @brief The commit handler
 * @see output_set_commit_handler() function of #tdm_func_display
 */
typedef void (*tdm_output_commit_handler)(tdm_output *output, unsigned int sequence,
										  unsigned int tv_sec, unsigned int tv_usec,
										  void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* _TDM_TYPES_H_ */
