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

#ifndef _TDM_HELPER_H_
#define _TDM_HELPER_H_

#include "tdm_types.h"
#include <tbm_surface.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file tdm_helper.h
 * @brief The header file to help a vendor to implement a backend module
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

#ifdef __cplusplus
}
#endif

#endif /* _TDM_HELPER_H_ */
