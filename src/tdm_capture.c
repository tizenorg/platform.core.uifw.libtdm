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
#include "tdm_backend.h"
#include "tdm_private.h"

#define CAPTURE_FUNC_ENTRY() \
    tdm_func_capture *func_capture; \
    tdm_private_display *private_display; \
    tdm_private_capture *private_capture; \
    tdm_error ret = TDM_ERROR_NONE; \
    TDM_RETURN_VAL_IF_FAIL(capture != NULL, TDM_ERROR_INVALID_PARAMETER); \
    private_capture = (tdm_private_capture*)capture; \
    private_display = private_capture->private_display; \
    func_capture = private_capture->func_capture

static void
_tdm_caputre_cb_done(tdm_capture *capture_backend, tdm_buffer *buffer, void *user_data)
{
    tdm_private_capture *private_capture = user_data;
    tdm_private_display *private_display = private_capture->private_display;
    int lock_after_cb_done = 0;
    int ret;

    ret = pthread_mutex_trylock(&private_display->lock);
    if (ret == 0)
        pthread_mutex_unlock(&private_display->lock);
    else  if (ret == EBUSY)
    {
        pthread_mutex_unlock(&private_display->lock);
        lock_after_cb_done = 1;
    }

    tdm_buffer_unref_backend(buffer);

    if (lock_after_cb_done)
        pthread_mutex_lock(&private_display->lock);
}

INTERN tdm_private_capture*
tdm_capture_create_output_internal(tdm_private_output *private_output, tdm_error *error)
{
    tdm_private_display *private_display;
    tdm_func_display *func_display;
    tdm_func_capture *func_capture;
    tdm_private_capture *private_capture = NULL;
    tdm_capture *capture_backend = NULL;
    tdm_error ret = TDM_ERROR_NONE;

    private_display = private_output->private_display;
    func_display = &private_display->func_display;
    func_capture = &private_display->func_capture;

    if (!(private_display->capabilities & TDM_DISPLAY_CAPABILITY_CAPTURE))
    {
        TDM_ERR("no capture capability");
        if (error)
            *error = TDM_ERROR_BAD_REQUEST;
        return NULL;
    }

    capture_backend = func_display->output_create_capture(private_output->output_backend, &ret);
    if (ret != TDM_ERROR_NONE)
    {
        if (error)
            *error = ret;
        return NULL;
    }

    private_capture = calloc(1, sizeof(tdm_private_capture));
    if (!private_capture)
    {
        TDM_ERR("failed: alloc memory");
        func_capture->capture_destroy(capture_backend);
        if (error)
            *error = TDM_ERROR_OUT_OF_MEMORY;
        return NULL;
    }

    ret = func_capture->capture_set_done_handler(capture_backend, _tdm_caputre_cb_done, private_capture);
    if (ret != TDM_ERROR_NONE)
    {
        TDM_ERR("set capture_done_handler failed");
        func_capture->capture_destroy(capture_backend);
        if (error)
            *error = ret;
        return NULL;
    }

    LIST_ADD(&private_capture->link, &private_output->capture_list);
    private_capture->func_capture = func_capture;
    private_capture->target = TDM_CAPTURE_TARGET_OUTPUT;
    private_capture->private_display = private_display;
    private_capture->private_output = private_output;
    private_capture->private_layer = NULL;
    private_capture->capture_backend = capture_backend;

    if (error)
        *error = TDM_ERROR_NONE;

    return private_capture;
}

INTERN tdm_private_capture*
tdm_capture_create_layer_internal(tdm_private_layer *private_layer, tdm_error *error)
{
    tdm_private_display *private_display;
    tdm_private_output *private_output;
    tdm_func_display *func_display;
    tdm_func_capture *func_capture;
    tdm_private_capture *private_capture = NULL;
    tdm_capture *capture_backend = NULL;
    tdm_error ret = TDM_ERROR_NONE;

    private_output = private_layer->private_output;
    private_display = private_output->private_display;
    func_display = &private_display->func_display;
    func_capture = &private_display->func_capture;

    if (!(private_display->capabilities & TDM_DISPLAY_CAPABILITY_CAPTURE))
    {
        TDM_ERR("no capture capability");
        if (error)
            *error = TDM_ERROR_BAD_REQUEST;
        return NULL;
    }

    capture_backend = func_display->layer_create_capture(private_layer->layer_backend, &ret);
    if (ret != TDM_ERROR_NONE)
        return NULL;

    private_capture = calloc(1, sizeof(tdm_private_capture));
    if (!private_capture)
    {
        TDM_ERR("failed: alloc memory");
        func_capture->capture_destroy(capture_backend);
        if (error)
            *error = TDM_ERROR_OUT_OF_MEMORY;
        return NULL;
    }

    LIST_ADD(&private_capture->link, &private_output->capture_list);
    private_capture->target = TDM_CAPTURE_TARGET_LAYER;
    private_capture->func_capture = func_capture;
    private_capture->private_display = private_display;
    private_capture->private_output = private_output;
    private_capture->private_layer = private_layer;
    private_capture->capture_backend = capture_backend;

    if (error)
        *error = TDM_ERROR_NONE;

    return private_capture;
}

INTERN void
tdm_capture_destroy_internal(tdm_private_capture *private_capture)
{
    tdm_func_capture *func_capture;

    if (!private_capture)
        return;

    LIST_DEL(&private_capture->link);

    func_capture = private_capture->func_capture;
    func_capture->capture_destroy(private_capture->capture_backend);

    free(private_capture);
}

EXTERN void
tdm_capture_destroy(tdm_capture *capture)
{
    tdm_private_capture *private_capture = capture;
    tdm_private_display *private_display;

    if (!private_capture)
        return;

    private_display = private_capture->private_display;

    pthread_mutex_lock(&private_display->lock);
    tdm_capture_destroy_internal(private_capture);
    pthread_mutex_unlock(&private_display->lock);
}

EXTERN tdm_error
tdm_capture_set_info(tdm_capture *capture, tdm_info_capture *info)
{
    CAPTURE_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(info != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    if (!func_capture->capture_set_info)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    ret = func_capture->capture_set_info(private_capture->capture_backend, info);

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_capture_attach(tdm_capture *capture, tdm_buffer *buffer)
{
    CAPTURE_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(buffer != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    if (!func_capture->capture_attach)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    tdm_buffer_ref_backend(buffer);
    ret = func_capture->capture_attach(private_capture->capture_backend,
                                       tdm_buffer_get_surface(buffer));

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_capture_commit(tdm_capture *capture)
{
    CAPTURE_FUNC_ENTRY();

    pthread_mutex_lock(&private_display->lock);

    if (!func_capture->capture_commit)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    ret = func_capture->capture_commit(private_capture->capture_backend);

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}
