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
 * @remark
 * tdm_helper_drm_fd is external drm_fd which is opened by ecore_drm.
 * This is very @b TRICKY!! But we have no choice at this time because ecore_drm
 * doesn't use tdm yet. When we make ecore_drm use tdm, tdm_helper_drm_fd will
 * be removed.
 * @warning
 * If tdm_helper_drm_fd is more than -1, a tdm backend module @b SHOULDN't call
 * drmWaitVBlank by itself because a DRM vblank event will be handled in ecore_drm
 * internally. In this case, a tdm backend module NEVER get a DRM vblank event.
 * If a tdm backend module need to handle a vendor specific DRM event,
 * drmAddUserHandler() of libdrm makes possible that a tdm backend module handle
 * it.
 * @par Example
 * @code
    static int
    _tdm_drm_user_handler(struct drm_event *event)
    {
        if (event->type != DRM_VENDOR_XXX_EVENT)
            return -1;

        //handling a vendor event

        return 0;
    }

    ...

    drm_data->drm_fd = -1;
    if (tdm_helper_drm_fd >= 0)
    {
        drm_data->drm_fd = tdm_helper_drm_fd;
        drmAddUserHandler(tdm_helper_drm_fd, _tdm_drm_user_handler);
    }

    if (drm_data->drm_fd < 0)
        drm_data->drm_fd = _tdm_drm_open_drm();

    ...

    drmRemoveUserHandler(tdm_helper_drm_fd, _tdm_drm_user_handler);
 * @endcode
 * @code
    if (tdm_helper_drm_fd == -1)
    {
        ...
        if (drmWaitVBlank(fd, &vbl))
            return TDM_ERROR_OPERATION_FAILED;
        ...
    }
 * @endcode
 * @endcode
 * @todo
 */
extern int tdm_helper_drm_fd;

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
void tdm_helper_dump_buffer(tbm_surface_h buffer, const char *file);

#ifdef __cplusplus
}
#endif

#endif /* _TDM_HELPER_H_ */
