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

#ifndef _TDM_DOC_H_
#define _TDM_DOC_H_

/**
 * @mainpage TDM
 * @author     Boram Park, boram1288.park@samsung.com
 * @date       Mar 17, 2016
 * @version    1.1.0
 * @par Introduction
 * TDM stands for Tizen Display Manager. It's the display HAL layer for tizen
 * display server. It offers the frontend APIs(@ref tdm.h) for a frontend user
 * and the abstraction interface(@ref tdm_backend.h) for a hardware vendor.
 * \n
 * @par Objects
 * TDM consists of display/output/layer/pp/capture objects. A frontend user can
 * get the output/layer/pp/capture hardware information with each object.
 * Basically, TDM supposes that all display devices have fixed outputs and
 * layers. A frontend user can get these outputs and layers with
 * #tdm_display_get_output_count, #tdm_display_get_output, #tdm_output_get_layer_count
 * and #tdm_output_get_layer. To get a pp/capture object, however, a frontend
 * user need to create a object with #tdm_display_create_pp, #tdm_output_create_capture
 * and #tdm_layer_create_capture if available.\n
 * \n
 * All changes of output/layer/pp/capture objects are applied when committed.
 * See #tdm_output_commit, #tdm_pp_commit and #tdm_capture_commit.
 * \n
 * @par Buffer management
 * TDM has its own buffer release mechanism to let an user know when a TDM buffer
 * becomes available for a next job. A frontend user can add a user release handler
 * to a TDM buffer with #tdm_buffer_add_release_handler, and this handler will be
 * called when it's disappered from screen or when pp/capture operation is done.
 * \n
 * @par Implementing a TDM backend module(for a hardware vendor)
 * A backend module @b SHOULD define the global data symbol of which name is
 * @b "tdm_backend_module_data". TDM will read this symbol, @b "tdm_backend_module_data",
 * at the initial time and call init() function of #tdm_backend_module.
 * A backend module at least @b SHOULD register the #tdm_func_display,
 * #tdm_func_output, #tdm_func_layer functions to a display object via
 * #tdm_backend_register_func_display(), #tdm_backend_register_func_output(),
 * #tdm_backend_register_func_layer() functions in initial time.\n
 * @code
    #include <tdm_backend.h>

    static tdm_func_display drm_func_display = {
        drm_display_get_capabilitiy,
        ...
    };

    static tdm_func_output drm_func_output = {
        drm_output_get_capability,
        ...
    };

    static tdm_func_layer drm_func_layer = {
        drm_layer_get_capability,
        ...
    };

    static tdm_drm_data *drm_data;

    tdm_backend_data*
    tdm_drm_init(tdm_display *dpy, tdm_error *error)
    {
        ...
        drm_data = calloc(1, sizeof(tdm_drm_data));
        ...
        ret = tdm_backend_register_func_display(dpy, &drm_func_display);
        if (ret != TDM_ERROR_NONE)
            goto failed;
        ret = tdm_backend_register_func_output(dpy, &drm_func_output);
        if (ret != TDM_ERROR_NONE)
            goto failed;
        ret = tdm_backend_register_func_layer(dpy, &drm_func_layer);
        if (ret != TDM_ERROR_NONE)
            goto failed;
        ...
        return (tdm_backend_data*)drm_data;
    }

    void
    tdm_drm_deinit(tdm_backend_data *bdata)
    {
        ...
        free(bdata);
    }

    tdm_backend_module tdm_backend_module_data =
    {
        "drm",
        "Samsung",
        TDM_BACKEND_ABI_VERSION,
        tdm_drm_init,
        tdm_drm_deinit
    };
 * @endcode
 * \n
 * A sample backend source code can be downloaded.
 * @code
 $ git clone ssh://{user_id}@review.tizen.org:29418/platform/core/uifw/libtdm-drm
 * @endcode
 * \n
 * After loading a TDM backend module, TDM will call @b display_get_capabilitiy(),
 * @b display_get_outputs(), @b output_get_capability(), @b output_get_layers(),
 * @b layer_get_capability() functions to get the hardware information. That is,
 * a TDM backend module @b SHOULD implement these 5 functions basically.\n
 * \n
 * In addition, if a hardware has a memory-to-memory converting device and the
 * capture device, a backend module can register the #tdm_func_pp functions and
 * #tdm_func_capture functions with #tdm_backend_register_func_pp() and
 * #tdm_backend_register_func_capture().\n
 * \n
 * The @b "Remarks" part of <tdm_backend.h> explains what things should be
 * careful for vendor to implement a TDM backend module.
 */

#endif /* _TDM_DOC_H_ */
