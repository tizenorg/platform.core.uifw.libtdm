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

#ifndef _TDM_HELPER_H_
#define _TDM_HELPER_H_

#include "tdm_types.h"
#include <tbm_surface.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file tdm_helper.h
 * @brief The header file to help tdm backend/frontend user
 */

/**
 * @brief Dump a buffer
 * @details
 * This function supports only if a buffer has below formats.
 * - TBM_FORMAT_ARGB8888
 * - TBM_FORMAT_XRGB8888
 * - TBM_FORMAT_YVU420
 * - TBM_FORMAT_YUV420
 * - TBM_FORMAT_NV12
 * - TBM_FORMAT_NV21
 * - TBM_FORMAT_YUYV
 * - TBM_FORMAT_UYVY
 * The filename extension should be "png" for TBM_FORMAT_ARGB8888 and TBM_FORMAT_XRGB8888
 * or "yuv" for YUV formats.
 * @param[in] buffer A TDM buffer
 * @param[in] file The path of file.
 */
void
tdm_helper_dump_buffer(tbm_surface_h buffer, const char *file);

/**
 * @brief Get a fd from the given enviroment variable.
 * @details
 * This function will dup the fd of the given enviroment variable. The Caller
 * @b SHOULD close the fd.
 * \n
 * In DRM system, a drm-master-fd @b SHOULD be shared between TDM backend and
 * TBM backend in display server side by using "TDM_DRM_MASTER_FD"
 * and "TBM_DRM_MASTER_FD".
 * @param[in] env The given enviroment variable
 * @return fd if success. Otherwise, -1.
 * @see #tdm_helper_set_fd()
 */
int tdm_helper_get_fd(const char *env);

/**
 * @brief Set the given fd to the give enviroment variable.
 * @details
 * In DRM system, a drm-master-fd @b SHOULD be shared between TDM backend and
 * TBM backend in display server side by using "TDM_DRM_MASTER_FD"
 * and "TBM_DRM_MASTER_FD".
 * @param[in] env The given enviroment variable
 * @param[in] fd The given fd
 * @see #tdm_helper_get_fd()
 */
void tdm_helper_set_fd(const char *env, int fd);

/**
 * @brief Start the dump debugging.
 * @details
 * Start tdm dump.
 * Make dump file when tdm_layer_set_buffer() function is called.
 * Set the dump count to 1.
 * @param[in] dumppath The given dump path
 * @param[in] count The dump count number
 * @see #tdm_helper_dump_stop()
 */
void
tdm_helper_dump_start(char *dumppath, int *count);

/**
 * @brief Stop the dump debugging.
 * @details
 * Stop tdm dump.
 * Set the dump count to 0.
 * @see #tdm_helper_dump_start()
 */
void
tdm_helper_dump_stop(void);

/**
 * @brief The tdm helper capture handler
 * @details
 * This handler will be called when composit image produced.
 * @see #tdm_helper_capture_output() function
 */
typedef void (*tdm_helper_capture_handler)(tbm_surface_h buffer, void *user_data);

/**
 * @brief Make an output's image surface.
 * @details Composit specific output's all layer's buffer to dst_buffer surface.
 * After composing, tdm_helper_capture_handler func will be called.
 * @param[in] output A output object
 * @param[in] dst_buffer A surface composite image saved
 * @param[in] x A horizontal position of composite image on dst_buffer
 * @param[in] y A vertical position of composite image on dst_buffer
 * @param[in] w A composite image width
 * @param[in] h A composite image height
 * @param[in] func A composing done handler
 * @param[in] user_data The user data
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_helper_capture_output(tdm_output *output, tbm_surface_h dst_buffer,
						  int x, int y, int w, int h,
						  tdm_helper_capture_handler func, void *data);

/**
 * @brief Fill the display information to the reply buffer as string.
 * @param[in] dpy A display object
 * @param[out] reply the string buffer to be filled by this function.
 * @param[out] len the length of the reply buffer
 */
void
tdm_helper_get_display_information(tdm_display *dpy, char *reply, int *len);

#ifdef __cplusplus
}
#endif

#endif /* _TDM_HELPER_H_ */
