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

#define COUNT_MAX   10

#define DISPLAY_FUNC_ENTRY() \
    tdm_private_display *private_display; \
    tdm_error ret = TDM_ERROR_NONE; /* default TDM_ERROR_NONE */\
    TDM_RETURN_VAL_IF_FAIL(dpy != NULL, TDM_ERROR_INVALID_PARAMETER); \
    private_display = (tdm_private_display*)dpy;

#define DISPLAY_FUNC_ENTRY_ERROR() \
    tdm_private_display *private_display; \
    tdm_error ret = TDM_ERROR_NONE; /* default TDM_ERROR_NONE */\
    TDM_RETURN_VAL_IF_FAIL_WITH_ERROR(dpy != NULL, TDM_ERROR_INVALID_PARAMETER, NULL); \
    private_display = (tdm_private_display*)dpy;

#define OUTPUT_FUNC_ENTRY() \
    tdm_private_display *private_display; \
    tdm_private_output *private_output; \
    tdm_error ret = TDM_ERROR_NONE; /* default TDM_ERROR_NONE */\
    TDM_RETURN_VAL_IF_FAIL(output != NULL, TDM_ERROR_INVALID_PARAMETER); \
    private_output = (tdm_private_output*)output; \
    private_display = private_output->private_display

#define OUTPUT_FUNC_ENTRY_ERROR() \
    tdm_private_display *private_display; \
    tdm_private_output *private_output; \
    tdm_error ret = TDM_ERROR_NONE; /* default TDM_ERROR_NONE */\
    TDM_RETURN_VAL_IF_FAIL_WITH_ERROR(output != NULL, TDM_ERROR_INVALID_PARAMETER, NULL); \
    private_output = (tdm_private_output*)output; \
    private_display = private_output->private_display

#define LAYER_FUNC_ENTRY() \
    tdm_private_display *private_display; \
    tdm_private_output *private_output; \
    tdm_private_layer *private_layer; \
    tdm_error ret = TDM_ERROR_NONE; /* default TDM_ERROR_NONE */\
    TDM_RETURN_VAL_IF_FAIL(layer != NULL, TDM_ERROR_INVALID_PARAMETER); \
    private_layer = (tdm_private_layer*)layer; \
    private_output = private_layer->private_output; \
    private_display = private_output->private_display

#define LAYER_FUNC_ENTRY_ERROR() \
    tdm_private_display *private_display; \
    tdm_private_output *private_output; \
    tdm_private_layer *private_layer; \
    tdm_error ret = TDM_ERROR_NONE; /* default TDM_ERROR_NONE */\
    TDM_RETURN_VAL_IF_FAIL_WITH_ERROR(layer != NULL, TDM_ERROR_INVALID_PARAMETER, NULL); \
    private_layer = (tdm_private_layer*)layer; \
    private_output = private_layer->private_output; \
    private_display = private_output->private_display

EXTERN tdm_error
tdm_display_get_capabilities(tdm_display *dpy, tdm_display_capability *capabilities)
{
    DISPLAY_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(capabilities != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    *capabilities = private_display->capabilities;

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_display_get_pp_capabilities(tdm_display *dpy, tdm_pp_capability *capabilities)
{
    DISPLAY_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(capabilities != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    *capabilities = private_display->caps_pp.capabilities;

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_display_get_pp_available_formats(tdm_display *dpy, const tbm_format **formats, int *count)
{
    DISPLAY_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(formats != NULL, TDM_ERROR_INVALID_PARAMETER);
    TDM_RETURN_VAL_IF_FAIL(count != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    *formats = (const tbm_format*)private_display->caps_pp.formats;
    *count = private_display->caps_pp.format_count;

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_display_get_pp_available_size(tdm_display *dpy, int *min_w, int *min_h,
                                  int *max_w, int *max_h, int *preferred_align)
{
    DISPLAY_FUNC_ENTRY();

    pthread_mutex_lock(&private_display->lock);

    if (min_w)
        *min_w = private_display->caps_pp.min_w;
    if (min_h)
        *min_h = private_display->caps_pp.min_h;
    if (max_w)
        *max_w = private_display->caps_pp.max_w;
    if (max_h)
        *max_h = private_display->caps_pp.max_h;
    if (preferred_align)
        *preferred_align = private_display->caps_pp.preferred_align;

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_display_get_capture_capabilities(tdm_display *dpy, tdm_capture_capability *capabilities)
{
    DISPLAY_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(capabilities != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    *capabilities = private_display->caps_capture.capabilities;

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_display_get_catpure_available_formats(tdm_display *dpy, const tbm_format **formats, int *count)
{
    DISPLAY_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(formats != NULL, TDM_ERROR_INVALID_PARAMETER);
    TDM_RETURN_VAL_IF_FAIL(count != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    *formats = (const tbm_format*)private_display->caps_capture.formats;
    *count = private_display->caps_capture.format_count;

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_display_get_output_count(tdm_display *dpy, int *count)
{
    tdm_private_output *private_output = NULL;

    DISPLAY_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(count != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    *count = 0;
    LIST_FOR_EACH_ENTRY(private_output, &private_display->output_list, link)
        (*count)++;

    if (*count == 0)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}


EXTERN tdm_output*
tdm_display_get_output(tdm_display *dpy, int index, tdm_error *error)
{
    tdm_private_output *private_output = NULL;
    int i = 0;

    DISPLAY_FUNC_ENTRY_ERROR();

    pthread_mutex_lock(&private_display->lock);

    if (error)
        *error = TDM_ERROR_NONE;

    i = 0;
    LIST_FOR_EACH_ENTRY(private_output, &private_display->output_list, link)
    {
        if (i == index)
        {
            pthread_mutex_unlock(&private_display->lock);
            return private_output;
        }
        i++;
    }

    pthread_mutex_unlock(&private_display->lock);

    return NULL;
}

EXTERN tdm_error
tdm_display_get_fd(tdm_display *dpy, int *fd)
{
    tdm_func_display *func_display;
    DISPLAY_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(fd != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    func_display = &private_display->func_display;

    if (!func_display->display_get_fd)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    ret = func_display->display_get_fd(private_display->bdata, fd);

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_display_handle_events(tdm_display *dpy)
{
    tdm_func_display *func_display;
    DISPLAY_FUNC_ENTRY();

    pthread_mutex_lock(&private_display->lock);

    func_display = &private_display->func_display;

    if (!func_display->display_handle_events)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    ret = func_display->display_handle_events(private_display->bdata);

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_pp*
tdm_display_create_pp(tdm_display *dpy, tdm_error *error)
{
    tdm_pp *pp;

    DISPLAY_FUNC_ENTRY_ERROR();

    pthread_mutex_lock(&private_display->lock);

    pp = (tdm_pp*)tdm_pp_create_internal(private_display, error);

    pthread_mutex_unlock(&private_display->lock);

    return pp;
}

EXTERN tdm_error
tdm_output_get_conn_status(tdm_output *output, tdm_output_conn_status *status)
{
    OUTPUT_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(status != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    *status = private_output->caps.status;

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_output_get_output_type(tdm_output *output, tdm_output_type *type)
{
    OUTPUT_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(type != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    *type = private_output->caps.type;

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_output_get_layer_count(tdm_output *output, int *count)
{
    tdm_private_layer *private_layer = NULL;

    OUTPUT_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(count != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    *count = 0;
    LIST_FOR_EACH_ENTRY(private_layer, &private_output->layer_list, link)
        (*count)++;
    if (*count == 0)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}


EXTERN tdm_layer*
tdm_output_get_layer(tdm_output *output, int index, tdm_error *error)
{
    tdm_private_layer *private_layer = NULL;
    int i = 0;

    OUTPUT_FUNC_ENTRY_ERROR();

    pthread_mutex_lock(&private_display->lock);

    if (error)
        *error = TDM_ERROR_NONE;

    LIST_FOR_EACH_ENTRY(private_layer, &private_output->layer_list, link)
    {
        if (i == index)
        {
            pthread_mutex_unlock(&private_display->lock);
            return private_layer;
        }
        i++;
    }

    pthread_mutex_unlock(&private_display->lock);

    return NULL;
}

EXTERN tdm_error
tdm_output_get_available_properties(tdm_output *output, const tdm_prop **props, int *count)
{
    OUTPUT_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(props != NULL, TDM_ERROR_INVALID_PARAMETER);
    TDM_RETURN_VAL_IF_FAIL(count != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    *props = (const tdm_prop*)private_output->caps.props;
    *count = private_output->caps.prop_count;

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_output_get_available_modes(tdm_output *output, const tdm_output_mode **modes, int *count)
{
    OUTPUT_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(modes != NULL, TDM_ERROR_INVALID_PARAMETER);
    TDM_RETURN_VAL_IF_FAIL(count != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    *modes = (const tdm_output_mode*)private_output->caps.modes;
    *count = private_output->caps.mode_count;

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_output_get_available_size(tdm_output *output, int *min_w, int *min_h,
                              int *max_w, int *max_h, int *preferred_align)
{
    OUTPUT_FUNC_ENTRY();

    pthread_mutex_lock(&private_display->lock);

    if (min_w)
        *min_w = private_output->caps.min_w;
    if (min_h)
        *min_h = private_output->caps.min_h;
    if (max_w)
        *max_w = private_output->caps.max_w;
    if (max_h)
        *max_h = private_output->caps.max_h;
    if (preferred_align)
        *preferred_align = private_output->caps.preferred_align;

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_output_get_physical_size(tdm_output *output, unsigned int *mmWidth, unsigned int *mmHeight)
{
    OUTPUT_FUNC_ENTRY();

    pthread_mutex_lock(&private_display->lock);

    if (mmWidth)
        *mmWidth = private_output->caps.mmWidth;
    if (mmHeight)
        *mmHeight = private_output->caps.mmHeight;

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_output_get_subpixel(tdm_output *output, unsigned int *subpixel)
{
    OUTPUT_FUNC_ENTRY();
    TDM_RETURN_VAL_IF_FAIL(subpixel != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    *subpixel = private_output->caps.subpixel;

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_output_get_pipe(tdm_output *output, unsigned int *pipe)
{
    OUTPUT_FUNC_ENTRY();
    TDM_RETURN_VAL_IF_FAIL(pipe != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    *pipe = private_output->pipe;

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}


EXTERN tdm_error
tdm_output_set_property(tdm_output *output, unsigned int id, tdm_value value)
{
    tdm_func_display *func_display;
    OUTPUT_FUNC_ENTRY();

    pthread_mutex_lock(&private_display->lock);

    func_display = &private_display->func_display;

    if (!func_display->output_set_property)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    ret = func_display->output_set_property(private_output->output_backend, id, value);

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_output_get_property(tdm_output *output, unsigned int id, tdm_value *value)
{
    tdm_func_display *func_display;
    OUTPUT_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(value != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    func_display = &private_display->func_display;

    if (!func_display->output_get_property)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    ret = func_display->output_get_property(private_output->output_backend, id, value);

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

static void
_tdm_output_cb_vblank(tdm_output *output_backend, unsigned int sequence,
                      unsigned int tv_sec, unsigned int tv_usec, void *user_data)
{
    tdm_private_vblank_handler *vblank_handler = user_data;
    tdm_private_display *private_display;

    TDM_RETURN_IF_FAIL(vblank_handler);

    private_display = vblank_handler->private_output->private_display;

    if (vblank_handler->func)
    {
        pthread_mutex_unlock(&private_display->lock);
        vblank_handler->func(vblank_handler->private_output, sequence,
                             tv_sec, tv_usec, vblank_handler->user_data);
        pthread_mutex_lock(&private_display->lock);
    }

    LIST_DEL(&vblank_handler->link);
    free(vblank_handler);
}

static void
_tdm_output_cb_commit(tdm_output *output_backend, unsigned int sequence,
                      unsigned int tv_sec, unsigned int tv_usec, void *user_data)
{
    tdm_private_commit_handler *commit_handler = user_data;
    tdm_private_display *private_display;
    tdm_private_output *private_output;
    tdm_private_layer *private_layer;

    TDM_RETURN_IF_FAIL(commit_handler);

    private_output = commit_handler->private_output;
    private_display = private_output->private_display;

    if (commit_handler->func)
    {
        pthread_mutex_unlock(&private_display->lock);
        commit_handler->func(private_output, sequence,
                             tv_sec, tv_usec, commit_handler->user_data);
        pthread_mutex_lock(&private_display->lock);
    }

    LIST_FOR_EACH_ENTRY(private_layer, &private_output->layer_list, link)
    {
        if (!private_layer->waiting_buffer)
            continue;

        if (private_layer->showing_buffer)
        {
            pthread_mutex_unlock(&private_display->lock);
            tdm_buffer_unref_backend(private_layer->showing_buffer);
            pthread_mutex_lock(&private_display->lock);
        }

        private_layer->showing_buffer = private_layer->waiting_buffer;
        private_layer->waiting_buffer = NULL;
    }

    LIST_DEL(&commit_handler->link);
    free(commit_handler);
}

EXTERN tdm_error
tdm_output_wait_vblank(tdm_output *output, int interval, int sync,
                       tdm_output_vblank_handler func, void *user_data)
{
    tdm_func_display *func_display;
    tdm_private_vblank_handler *vblank_handler;
    OUTPUT_FUNC_ENTRY();

    pthread_mutex_lock(&private_display->lock);

    func_display = &private_display->func_display;

    if (!func_display->output_wait_vblank)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    vblank_handler = calloc(1, sizeof(tdm_private_vblank_handler));
    if (!vblank_handler)
    {
        TDM_ERR("failed: alloc memory");
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_OUT_OF_MEMORY;
    }

    LIST_ADD(&vblank_handler->link, &private_output->vblank_handler_list);
    vblank_handler->private_output = private_output;
    vblank_handler->func = func;
    vblank_handler->user_data = user_data;

    ret = func_display->output_wait_vblank(private_output->output_backend, interval,
                                           sync, vblank_handler);
    if (ret != TDM_ERROR_NONE)
    {
        pthread_mutex_unlock(&private_display->lock);
        return ret;
    }

    if (!private_output->regist_vblank_cb)
    {
        private_output->regist_vblank_cb = 1;
        ret = func_display->output_set_vblank_handler(private_output->output_backend,
                                                      _tdm_output_cb_vblank);
    }

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_output_commit(tdm_output *output, int sync, tdm_output_commit_handler func, void *user_data)
{
    tdm_func_display *func_display;
    tdm_private_commit_handler *commit_handler;
    OUTPUT_FUNC_ENTRY();

    pthread_mutex_lock(&private_display->lock);

    func_display = &private_display->func_display;

    if (!func_display->output_commit)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    commit_handler = calloc(1, sizeof(tdm_private_commit_handler));
    if (!commit_handler)
    {
        TDM_ERR("failed: alloc memory");
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_OUT_OF_MEMORY;
    }

    LIST_ADD(&commit_handler->link, &private_output->commit_handler_list);
    commit_handler->private_output = private_output;
    commit_handler->func = func;
    commit_handler->user_data = user_data;

    ret = func_display->output_commit(private_output->output_backend, sync, commit_handler);
    if (ret != TDM_ERROR_NONE)
    {
        pthread_mutex_unlock(&private_display->lock);
        return ret;
    }

    if (!private_output->regist_commit_cb)
    {
        private_output->regist_commit_cb = 1;
        ret = func_display->output_set_commit_handler(private_output->output_backend,
                                                      _tdm_output_cb_commit);
    }

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_output_set_mode(tdm_output *output, const tdm_output_mode *mode)
{
    tdm_func_display *func_display;
    OUTPUT_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(mode != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    func_display = &private_display->func_display;

    if (!func_display->output_set_mode)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    ret = func_display->output_set_mode(private_output->output_backend, mode);

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_output_get_mode(tdm_output *output, const tdm_output_mode **mode)
{
    tdm_func_display *func_display;
    OUTPUT_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(mode != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    func_display = &private_display->func_display;

    if (!func_display->output_get_mode)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    ret = func_display->output_get_mode(private_output->output_backend, mode);

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_output_set_dpms(tdm_output *output, tdm_output_dpms dpms_value)
{
    tdm_func_display *func_display;
    OUTPUT_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(dpms_value >= TDM_OUTPUT_DPMS_ON, TDM_ERROR_INVALID_PARAMETER);
    TDM_RETURN_VAL_IF_FAIL(dpms_value < TDM_OUTPUT_DPMS_MAX, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    func_display = &private_display->func_display;

    if (!func_display->output_set_dpms)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    ret = func_display->output_set_dpms(private_output->output_backend, dpms_value);

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_output_get_dpms(tdm_output *output, tdm_output_dpms *dpms_value)
{
    tdm_func_display *func_display;
    OUTPUT_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(dpms_value != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    func_display = &private_display->func_display;

    if (!func_display->output_get_dpms)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    ret = func_display->output_get_dpms(private_output->output_backend, dpms_value);

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_capture*
tdm_output_create_capture(tdm_output *output, tdm_error *error)
{
    tdm_capture *capture = NULL;

    OUTPUT_FUNC_ENTRY_ERROR();

    pthread_mutex_lock(&private_display->lock);

    capture = (tdm_capture*)tdm_capture_create_output_internal(private_output, error);

    pthread_mutex_unlock(&private_display->lock);

    return capture;
}

EXTERN tdm_error
tdm_layer_get_capabilities(tdm_layer *layer, tdm_layer_capability *capabilities)
{
    LAYER_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(capabilities != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    *capabilities = private_layer->caps.capabilities;

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_layer_get_available_formats(tdm_layer *layer, const tbm_format **formats, int *count)
{
    LAYER_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(formats != NULL, TDM_ERROR_INVALID_PARAMETER);
    TDM_RETURN_VAL_IF_FAIL(count != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    *formats = (const tbm_format*)private_layer->caps.formats;
    *count = private_layer->caps.format_count;

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_layer_get_available_properties(tdm_layer *layer, const tdm_prop **props, int *count)
{
    LAYER_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(props != NULL, TDM_ERROR_INVALID_PARAMETER);
    TDM_RETURN_VAL_IF_FAIL(count != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    *props = (const tdm_prop*)private_layer->caps.props;
    *count = private_layer->caps.prop_count;

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_layer_get_zpos(tdm_layer *layer, unsigned int *zpos)
{
    LAYER_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(zpos != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    *zpos = private_layer->caps.zpos;

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_layer_set_property(tdm_layer *layer, unsigned int id, tdm_value value)
{
    tdm_func_display *func_display;
    LAYER_FUNC_ENTRY();

    pthread_mutex_lock(&private_display->lock);

    func_display = &private_display->func_display;

    if (!func_display->layer_set_property)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    ret = func_display->layer_set_property(private_layer->layer_backend, id, value);

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_layer_get_property(tdm_layer *layer, unsigned int id, tdm_value *value)
{
    tdm_func_display *func_display;
    LAYER_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(value != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    func_display = &private_display->func_display;

    if (!func_display->layer_get_property)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    ret = func_display->layer_get_property(private_layer->layer_backend, id, value);

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_layer_set_info(tdm_layer *layer, tdm_info_layer *info)
{
    tdm_func_display *func_display;
    LAYER_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(info != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    func_display = &private_display->func_display;

    private_layer->usable = 0;

    if (!func_display->layer_set_info)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    ret = func_display->layer_set_info(private_layer->layer_backend, info);

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_layer_get_info(tdm_layer *layer, tdm_info_layer *info)
{
    tdm_func_display *func_display;
    LAYER_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(info != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    func_display = &private_display->func_display;

    if (!func_display->layer_get_info)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    ret = func_display->layer_get_info(private_layer->layer_backend, info);

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_layer_set_buffer(tdm_layer *layer, tdm_buffer *buffer)
{
    tdm_func_display *func_display;
    LAYER_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(buffer != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    func_display = &private_display->func_display;

    private_layer->usable = 0;

    if (!func_display->layer_set_buffer)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    if (private_layer->waiting_buffer)
    {
        pthread_mutex_unlock(&private_display->lock);
        tdm_buffer_unref_backend(private_layer->waiting_buffer);
        pthread_mutex_lock(&private_display->lock);
    }

    private_layer->waiting_buffer = tdm_buffer_ref_backend(buffer);

    ret = func_display->layer_set_buffer(private_layer->layer_backend, buffer);

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_layer_unset_buffer(tdm_layer *layer)
{
    tdm_func_display *func_display;
    LAYER_FUNC_ENTRY();

    pthread_mutex_lock(&private_display->lock);

    func_display = &private_display->func_display;

    if (private_layer->waiting_buffer)
    {
        pthread_mutex_unlock(&private_display->lock);
        tdm_buffer_unref_backend(private_layer->waiting_buffer);
        pthread_mutex_lock(&private_display->lock);
        private_layer->waiting_buffer = NULL;
    }

    if (private_layer->showing_buffer)
    {
        pthread_mutex_unlock(&private_display->lock);
        tdm_buffer_unref_backend(private_layer->showing_buffer);
        pthread_mutex_lock(&private_display->lock);
        private_layer->showing_buffer = NULL;
    }

    private_layer->usable = 1;

    if (!func_display->layer_unset_buffer)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    ret = func_display->layer_unset_buffer(private_layer->layer_backend);

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_layer_is_usable(tdm_layer *layer, unsigned int *usable)
{
    LAYER_FUNC_ENTRY();

    TDM_RETURN_VAL_IF_FAIL(usable != NULL, TDM_ERROR_INVALID_PARAMETER);

    pthread_mutex_lock(&private_display->lock);

    *usable = private_layer->usable;

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_error
tdm_layer_set_video_pos(tdm_layer *layer, int zpos)
{
    tdm_func_display *func_display;
    LAYER_FUNC_ENTRY();

    pthread_mutex_lock(&private_display->lock);

    func_display = &private_display->func_display;

    if (!(private_layer->caps.capabilities & TDM_LAYER_CAPABILITY_VIDEO))
    {
        TDM_ERR("layer is not video layer");
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_INVALID_PARAMETER;
    }

    if (!func_display->layer_set_video_pos)
    {
        pthread_mutex_unlock(&private_display->lock);
        return TDM_ERROR_NONE;
    }

    ret = func_display->layer_set_video_pos(private_layer->layer_backend, zpos);

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

EXTERN tdm_capture*
tdm_layer_create_capture(tdm_layer *layer, tdm_error *error)
{
    tdm_capture *capture = NULL;

    LAYER_FUNC_ENTRY_ERROR();

    pthread_mutex_lock(&private_display->lock);

    capture = (tdm_capture*)tdm_capture_create_layer_internal(private_layer, error);

    pthread_mutex_unlock(&private_display->lock);

    return capture;
}
