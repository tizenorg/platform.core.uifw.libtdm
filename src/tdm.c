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

static tdm_private_layer*
_tdm_display_find_private_layer(tdm_private_output *private_output, tdm_layer *layer)
{
    tdm_private_layer *private_layer = NULL;

    LIST_FOR_EACH_ENTRY(private_layer, &private_output->layer_list, link)
    {
        if (private_layer->layer == layer)
            return private_layer;
    }

    return NULL;
}

static tdm_private_output*
_tdm_display_find_private_output(tdm_private_display *private_display, tdm_output *output)
{
    tdm_private_output *private_output = NULL;

    LIST_FOR_EACH_ENTRY(private_output, &private_display->output_list, link)
    {
        if (private_output->output == output)
            return private_output;
    }

    return NULL;
}

static void
_tdm_display_destroy_caps_pp(tdm_caps_pp *caps_pp)
{
    free(caps_pp->formats);
    memset(caps_pp, 0, sizeof(tdm_caps_pp));
}

static void
_tdm_display_destroy_caps_capture(tdm_caps_capture *caps_capture)
{
    free(caps_capture->formats);
    memset(caps_capture, 0, sizeof(tdm_caps_capture));
}

static void
_tdm_display_destroy_caps_layer(tdm_caps_layer *caps_layer)
{
    free(caps_layer->formats);
    free(caps_layer->props);
    memset(caps_layer, 0, sizeof(tdm_caps_layer));
}

static void
_tdm_display_destroy_caps_output(tdm_caps_output *caps_output)
{
    free(caps_output->modes);
    free(caps_output->props);
    memset(caps_output, 0, sizeof(tdm_caps_output));
}

static void
_tdm_display_destroy_private_layer(tdm_private_layer *private_layer)
{
    tdm_private_capture *c = NULL, *cc = NULL;

    LIST_DEL(&private_layer->link);

    LIST_FOR_EACH_ENTRY_SAFE(c, cc, &private_layer->capture_list, link)
        tdm_capture_destroy_internal(c);

    _tdm_display_destroy_caps_layer(&private_layer->caps);

    free(private_layer);
}

static void
_tdm_display_destroy_private_output(tdm_private_output *private_output)
{
    tdm_private_layer *l = NULL, *ll = NULL;
    tdm_private_capture *c = NULL, *cc = NULL;
    tdm_private_vblank_handler *v = NULL, *vv = NULL;
    tdm_private_commit_handler *m = NULL, *mm = NULL;

    LIST_DEL(&private_output->link);

    free(private_output->layers_ptr);

    LIST_FOR_EACH_ENTRY_SAFE(v, vv, &private_output->vblank_handler_list, link)
    {
        LIST_DEL(&v->link);
        free(v);
    }

    LIST_FOR_EACH_ENTRY_SAFE(m, mm, &private_output->commit_handler_list, link)
    {
        LIST_DEL(&m->link);
        free(m);
    }

    LIST_FOR_EACH_ENTRY_SAFE(c, cc, &private_output->capture_list, link)
        tdm_capture_destroy_internal(c);

    LIST_FOR_EACH_ENTRY_SAFE(l, ll, &private_output->layer_list, link)
        _tdm_display_destroy_private_layer(l);

    _tdm_display_destroy_caps_output(&private_output->caps);

    free(private_output);
}

static void
_tdm_display_destroy_private_display(tdm_private_display *private_display)
{
    tdm_private_output *o = NULL, *oo = NULL;
    tdm_private_pp *p = NULL, *pp = NULL;

    free(private_display->outputs_ptr);

    LIST_FOR_EACH_ENTRY_SAFE(p, pp, &private_display->pp_list, link)
        tdm_pp_destroy_internal(p);

    LIST_FOR_EACH_ENTRY_SAFE(o, oo, &private_display->output_list, link)
        _tdm_display_destroy_private_output(o);

    _tdm_display_destroy_caps_pp(&private_display->caps_pp);
    _tdm_display_destroy_caps_capture(&private_display->caps_capture);
    private_display->caps_display.capabilities = 0;
}

static tdm_error
_tdm_display_update_caps_pp(tdm_private_display *private_display, tdm_caps_pp *caps)
{
    tdm_func_display *func_display = &private_display->func_display;
    char buf[1024];
    int bufsize = sizeof(buf);
    char *str_buf = buf;
    int *len_buf = &bufsize;
    int i;
    tdm_error ret;

    if (!(private_display->caps_display.capabilities & TDM_DISPLAY_CAPABILITY_PP))
        return TDM_ERROR_NONE;

    if (!func_display->display_get_pp_capability)
    {
        TDM_ERR("no display_get_pp_capability()");
        return TDM_ERROR_BAD_MODULE;
    }

    ret = func_display->display_get_pp_capability(private_display->bdata, caps);
    if (ret != TDM_ERROR_NONE)
    {
        TDM_ERR("display_get_pp_capability() failed");
        return TDM_ERROR_BAD_MODULE;
    }

    TDM_DBG("pp capabilities: %x", caps->capabilities);
    buf[0] = '\0';
    for (i = 0; i < caps->format_count; i++)
        TDM_SNPRINTF(str_buf, len_buf, "%c%c%c%c ", FOURCC_STR(caps->formats[i]));
    TDM_DBG("pp formats: %s", buf);
    TDM_DBG("pp min  : %dx%d", caps->min_w, caps->min_h);
    TDM_DBG("pp max  : %dx%d", caps->max_w, caps->max_h);
    TDM_DBG("pp align: %d", caps->preferred_align);

    return TDM_ERROR_NONE;
}

static tdm_error
_tdm_display_update_caps_capture(tdm_private_display *private_display, tdm_caps_capture *caps)
{
    tdm_func_display *func_display = &private_display->func_display;
    char buf[1024];
    int bufsize = sizeof(buf);
    char *str_buf = buf;
    int *len_buf = &bufsize;
    int i;
    tdm_error ret;

    if (!(private_display->caps_display.capabilities & TDM_DISPLAY_CAPABILITY_CAPTURE))
        return TDM_ERROR_NONE;

    if (!func_display->display_get_capture_capability)
    {
        TDM_ERR("no display_get_capture_capability()");
        return TDM_ERROR_BAD_MODULE;
    }

    ret = func_display->display_get_capture_capability(private_display->bdata, caps);
    if (ret != TDM_ERROR_NONE)
    {
        TDM_ERR("display_get_capture_capability() failed");
        return TDM_ERROR_BAD_MODULE;
    }

    buf[0] = '\0';
    for (i = 0; i < caps->format_count; i++)
        TDM_SNPRINTF(str_buf, len_buf, "%c%c%c%c ", FOURCC_STR(caps->formats[i]));
    TDM_DBG("capture formats: %s", buf);

    return TDM_ERROR_NONE;
}

static tdm_error
_tdm_display_update_caps_layer(tdm_private_display *private_display, tdm_layer *layer, tdm_caps_layer *caps)
{
    tdm_func_display *func_display = &private_display->func_display;
    char buf[1024];
    int bufsize = sizeof(buf);
    char *str_buf = buf;
    int *len_buf = &bufsize;
    int i;
    tdm_error ret;

    if (!func_display->layer_get_capability)
    {
        TDM_ERR("no layer_get_capability()");
        return TDM_ERROR_BAD_MODULE;
    }

    ret = func_display->layer_get_capability(layer, caps);
    if (ret != TDM_ERROR_NONE)
    {
        TDM_ERR("layer_get_capability() failed");
        return TDM_ERROR_BAD_MODULE;
    }

    TDM_DBG("layer capabilities: %x", caps->capabilities);
    TDM_DBG("layer zpos : %d", caps->zpos);
    buf[0] = '\0';
    for (i = 0; i < caps->format_count; i++)
        TDM_SNPRINTF(str_buf, len_buf, "%c%c%c%c ", FOURCC_STR(caps->formats[i]));
    TDM_DBG("layer formats: %s", buf);
    for (i = 0; i < caps->prop_count; i++)
        TDM_DBG("layer props: %d, %s", caps->props[i].id, caps->props[i].name);

    return TDM_ERROR_NONE;
}

static tdm_error
_tdm_display_update_caps_output(tdm_private_display *private_display, tdm_output *output, tdm_caps_output *caps)
{
    tdm_func_display *func_display = &private_display->func_display;
    int i;
    tdm_error ret;

    if (!func_display->output_get_capability)
    {
        TDM_ERR("no output_get_capability()");
        return TDM_ERROR_BAD_MODULE;
    }

    ret = func_display->output_get_capability(output, caps);
    if (ret != TDM_ERROR_NONE)
    {
        TDM_ERR("output_get_capability() failed");
        return TDM_ERROR_BAD_MODULE;
    }

    TDM_DBG("output status: %d", caps->status);
    TDM_DBG("output type : %d", caps->type);
    for (i = 0; i < caps->prop_count; i++)
        TDM_DBG("output props: %d, %s", caps->props[i].id, caps->props[i].name);
    for (i = 0; i < caps->mode_count; i++)
        TDM_DBG("output modes: name(%s), size(%dx%d), refresh(%d), flags(%d), type(%d)",
                 caps->modes[i]->name, caps->modes[i]->width, caps->modes[i]->height,
                 caps->modes[i]->refresh, caps->modes[i]->flags, caps->modes[i]->type);
    TDM_DBG("output min  : %dx%d", caps->min_w, caps->min_h);
    TDM_DBG("output max  : %dx%d", caps->max_w, caps->max_h);
    TDM_DBG("output align: %d", caps->preferred_align);

    return TDM_ERROR_NONE;
}

static tdm_error
_tdm_display_update_layer(tdm_private_display *private_display,
                          tdm_private_output *private_output,
                          tdm_layer *layer)
{
    tdm_private_layer *private_layer;
    tdm_error ret;

    private_layer = _tdm_display_find_private_layer(private_output, layer);
    if (!private_layer)
    {
        private_layer = calloc(1, sizeof(tdm_private_layer));
        TDM_RETURN_VAL_IF_FAIL(private_layer != NULL, TDM_ERROR_OUT_OF_MEMORY);

        LIST_ADD(&private_layer->link, &private_output->layer_list);
        private_layer->func_display = &private_display->func_display;
        private_layer->private_display = private_display;
        private_layer->private_output = private_output;
        private_layer->layer = layer;

        LIST_INITHEAD(&private_layer->capture_list);

        private_layer->usable = 1;
    }
    else
        _tdm_display_destroy_caps_layer(&private_layer->caps);

    ret = _tdm_display_update_caps_layer(private_display, layer, &private_layer->caps);
    if (ret != TDM_ERROR_NONE)
        goto failed_update;

    return TDM_ERROR_NONE;
failed_update:
    _tdm_display_destroy_private_layer(private_layer);
    return ret;
}

static tdm_error
_tdm_display_update_output(tdm_private_display *private_display, tdm_output *output, int pipe)
{
    tdm_func_display *func_display = &private_display->func_display;
    tdm_private_output *private_output = NULL;
    tdm_layer **layers = NULL;
    int layer_count = 0, i;
    tdm_error ret;

    private_output = _tdm_display_find_private_output(private_display, output);
    if (!private_output)
    {
        private_output = calloc(1, sizeof(tdm_private_output));
        TDM_RETURN_VAL_IF_FAIL(private_output != NULL, TDM_ERROR_OUT_OF_MEMORY);

        LIST_ADD(&private_output->link, &private_display->output_list);
        private_output->func_display = func_display;
        private_output->private_display = private_display;
        private_output->output = output;
        private_output->pipe = pipe;

        LIST_INITHEAD(&private_output->layer_list);
        LIST_INITHEAD(&private_output->capture_list);
        LIST_INITHEAD(&private_output->vblank_handler_list);
        LIST_INITHEAD(&private_output->commit_handler_list);
    }
    else
        _tdm_display_destroy_caps_output(&private_output->caps);

    ret = _tdm_display_update_caps_output(private_display, output, &private_output->caps);
    if (ret != TDM_ERROR_NONE)
        return ret;

    layers = func_display->output_get_layers(output, &layer_count, &ret);
    if (ret != TDM_ERROR_NONE)
        goto failed_update;

    for (i = 0; i < layer_count; i++)
    {
        ret = _tdm_display_update_layer(private_display, private_output, layers[i]);
        if (ret != TDM_ERROR_NONE)
            goto failed_update;
    }

    free(layers);

    return TDM_ERROR_NONE;
failed_update:
    _tdm_display_destroy_private_output(private_output);
    free(layers);
    return ret;
}

static tdm_error
_tdm_display_update_internal(tdm_private_display *private_display, int only_display)
{
    tdm_func_display *func_display = &private_display->func_display;
    tdm_output **outputs = NULL;
    int output_count = 0, i;
    tdm_error ret;

    LIST_INITHEAD(&private_display->output_list);
    LIST_INITHEAD(&private_display->pp_list);

    if (!only_display)
    {
        ret = _tdm_display_update_caps_pp(private_display, &private_display->caps_pp);
        if (ret != TDM_ERROR_NONE)
            goto failed_update;

        ret = _tdm_display_update_caps_capture(private_display, &private_display->caps_capture);
        if (ret != TDM_ERROR_NONE)
            goto failed_update;
    }

    outputs = func_display->display_get_outputs(private_display->bdata, &output_count, &ret);
    if (ret != TDM_ERROR_NONE)
        goto failed_update;

    for (i = 0; i < output_count; i++)
    {
        ret = _tdm_display_update_output(private_display, outputs[i], i);
        if (ret != TDM_ERROR_NONE)
            goto failed_update;
    }

    free(outputs);

    return TDM_ERROR_NONE;

failed_update:
    _tdm_display_destroy_private_display(private_display);
    free(outputs);
    return ret;
}


EXTERN tdm_error
tdm_display_update(tdm_display *dpy)
{
    tdm_private_display *private_display;
    tdm_error ret;

    TDM_RETURN_VAL_IF_FAIL(dpy != NULL, TDM_ERROR_INVALID_PARAMETER);

    private_display = dpy;
    pthread_mutex_lock(&private_display->lock);

    ret = _tdm_display_update_internal(private_display, 1);

    pthread_mutex_unlock(&private_display->lock);

    return ret;
}

#define SUFFIX_MODULE    ".so"
#define DEFAULT_MODULE   "libtdm-default"SUFFIX_MODULE

int tdm_debug;

static tdm_private_display *g_private_display;
static pthread_mutex_t gLock = PTHREAD_MUTEX_INITIALIZER;

static tdm_error
_tdm_display_check_module(tdm_backend_module *module)
{
    const char *name;
    const char *vendor;
    int major, minor;
    int abimaj, abimin;

    abimaj = TDM_BACKEND_GET_ABI_MAJOR(TDM_BACKEND_ABI_VERSION);
    abimin = TDM_BACKEND_GET_ABI_MINOR(TDM_BACKEND_ABI_VERSION);

    TDM_INFO("TDM module ABI version : %d.%d", abimaj, abimin);

    name = module->name ? module->name : "unknown";
    vendor = module->vendor ? module->vendor : "unknown";
    major = TDM_BACKEND_GET_ABI_MAJOR(module->abi_version);
    minor = TDM_BACKEND_GET_ABI_MINOR(module->abi_version);

    TDM_INFO("TDM module name: %s", name);
    TDM_INFO("'%s' vendor: %s", name, vendor);
    TDM_INFO("'%s' version: %d.%d", name, major, minor);

    if (major != abimaj)
    {
        TDM_ERR("'%s' major version mismatch, %d != %d", name, major, abimaj);
        return TDM_ERROR_BAD_MODULE;
    }

    if (minor > abimin)
    {
        TDM_ERR("'%s' minor version(%d) is newer than %d", name, minor, abimin);
        return TDM_ERROR_BAD_MODULE;
    }

    if (!module->init)
    {
        TDM_ERR("'%s' doesn't have init function", name);
        return TDM_ERROR_BAD_MODULE;
    }

    if (!module->deinit)
    {
        TDM_ERR("'%s' doesn't have deinit function", name);
        return TDM_ERROR_BAD_MODULE;
    }

    return TDM_ERROR_NONE;
}

static tdm_error
_tdm_display_check_backend_functions(tdm_private_display *private_display)
{
    tdm_func_display *func_display = &private_display->func_display;
    tdm_error ret;

    /* below functions should be implemented in backend side */

    TDM_RETURN_VAL_IF_FAIL(func_display != NULL, TDM_ERROR_BAD_MODULE);
    TDM_RETURN_VAL_IF_FAIL(func_display->display_get_capabilitiy, TDM_ERROR_BAD_MODULE);
    TDM_RETURN_VAL_IF_FAIL(func_display->display_get_outputs, TDM_ERROR_BAD_MODULE);
    TDM_RETURN_VAL_IF_FAIL(func_display->output_get_capability, TDM_ERROR_BAD_MODULE);
    TDM_RETURN_VAL_IF_FAIL(func_display->output_get_layers, TDM_ERROR_BAD_MODULE);
    TDM_RETURN_VAL_IF_FAIL(func_display->layer_get_capability, TDM_ERROR_BAD_MODULE);

    ret = func_display->display_get_capabilitiy(private_display->bdata,
                                                &private_display->caps_display);
    if (ret != TDM_ERROR_NONE)
    {
        TDM_ERR("display_get_capabilitiy() failed");
        return TDM_ERROR_BAD_MODULE;
    }

    if (private_display->caps_display.capabilities & TDM_DISPLAY_CAPABILITY_PP)
    {
        tdm_func_pp *func_pp = &private_display->func_pp;
        TDM_RETURN_VAL_IF_FAIL(func_display->display_get_pp_capability, TDM_ERROR_BAD_MODULE);
        TDM_RETURN_VAL_IF_FAIL(func_display->display_create_pp, TDM_ERROR_BAD_MODULE);
        TDM_RETURN_VAL_IF_FAIL(func_pp->pp_destroy, TDM_ERROR_BAD_MODULE);
        TDM_RETURN_VAL_IF_FAIL(func_pp->pp_commit, TDM_ERROR_BAD_MODULE);
        TDM_RETURN_VAL_IF_FAIL(func_pp->pp_set_done_handler, TDM_ERROR_BAD_MODULE);
    }

    if (private_display->caps_display.capabilities & TDM_DISPLAY_CAPABILITY_CAPTURE)
    {
        tdm_func_capture *func_capture = &private_display->func_capture;
        TDM_RETURN_VAL_IF_FAIL(func_display->display_get_capture_capability, TDM_ERROR_BAD_MODULE);
        TDM_RETURN_VAL_IF_FAIL(func_display->output_create_capture, TDM_ERROR_BAD_MODULE);
        TDM_RETURN_VAL_IF_FAIL(func_display->layer_create_capture, TDM_ERROR_BAD_MODULE);
        TDM_RETURN_VAL_IF_FAIL(func_capture->capture_destroy, TDM_ERROR_BAD_MODULE);
        TDM_RETURN_VAL_IF_FAIL(func_capture->capture_commit, TDM_ERROR_BAD_MODULE);
        TDM_RETURN_VAL_IF_FAIL(func_capture->capture_set_done_handler, TDM_ERROR_BAD_MODULE);
    }

    return TDM_ERROR_NONE;
}

static tdm_error
_tdm_display_load_module_with_file(tdm_private_display *private_display, const char *file)
{
    char path[PATH_MAX] = {0,};
    tdm_backend_module *module_data;
    void *module;
    tdm_error ret;

    snprintf(path, sizeof(path), TDM_MODULE_PATH "/%s", file);

    module = dlopen(path, RTLD_LAZY);
    if (!module)
    {
        TDM_ERR("failed to load module: %s(%s)", dlerror(), file);
        return TDM_ERROR_BAD_MODULE;
    }

    module_data = dlsym(module, "tdm_backend_module_data");
    if (!module_data)
    {
        TDM_ERR("'%s' doesn't have data object", file);
        ret = TDM_ERROR_BAD_MODULE;
        goto failed_load;
    }

    /* check if version, init() and deinit() are valid or not */
    ret = _tdm_display_check_module(module_data);
    if (ret != TDM_ERROR_NONE)
        goto failed_load;

    /* We don't care if backend_data is NULL or not. It's up to backend. */
    private_display->bdata = module_data->init((tdm_display*)private_display, &ret);
    if (ret != TDM_ERROR_NONE)
    {
        TDM_ERR("'%s' init failed", file);
        goto failed_load;
    }

    ret = _tdm_display_check_backend_functions(private_display);
    if (ret != TDM_ERROR_NONE)
    {
        module_data->deinit(private_display->bdata);
        private_display->bdata = NULL;
        goto failed_load;
    }

    private_display->module_data = module_data;
    private_display->module = module;

    TDM_INFO("Success to load module(%s)", file);

    return TDM_ERROR_NONE;
failed_load:
    dlclose(module);
    return ret;
}

static tdm_error
_tdm_display_load_module(tdm_private_display *private_display)
{
    const char *module_name;
    struct dirent **namelist;
    int n;
    tdm_error ret = 0;

    module_name = getenv("TDM_MODULE");
    if (!module_name)
        module_name = DEFAULT_MODULE;

    /* load bufmgr priv from default lib */
    ret = _tdm_display_load_module_with_file(private_display, module_name);
    if (ret == TDM_ERROR_NONE)
        return TDM_ERROR_NONE;

    /* load bufmgr priv from configured path */
    n = scandir(TDM_MODULE_PATH, &namelist, 0, alphasort);
    if (n < 0)
    {
        TDM_ERR("no module in '%s'\n", TDM_MODULE_PATH);
        return TDM_ERROR_BAD_MODULE;
    }

    ret = TDM_ERROR_BAD_MODULE;
    while (n--)
    {
        if (ret < 0 && strstr(namelist[n]->d_name, SUFFIX_MODULE))
            ret = _tdm_display_load_module_with_file(private_display, namelist[n]->d_name);

        free(namelist[n]);
    }
    free(namelist);

    return ret;
}

static void
_tdm_display_unload_module(tdm_private_display *private_display)
{
    if (private_display->module_data)
        private_display->module_data->deinit(private_display->bdata);
    if (private_display->module)
        dlclose(private_display->module);

    private_display->bdata = NULL;
    private_display->module_data = NULL;
    private_display->module = NULL;
}

EXTERN tdm_display*
tdm_display_init(tdm_error *error)
{
    tdm_private_display *private_display = NULL;
    const char *debug;
    tdm_error ret;

    pthread_mutex_lock(&gLock);

    if (g_private_display)
    {
        g_private_display->init_count++;
        pthread_mutex_unlock(&gLock);
        if (error)
            *error = TDM_ERROR_NONE;
        return g_private_display;
    }

    debug = getenv("TDM_DEBUG");
    if (debug && (strstr(debug, "1")))
        tdm_debug = 1;

    private_display = calloc(1, sizeof(tdm_private_display));
    if (!private_display)
    {
        ret = TDM_ERROR_OUT_OF_MEMORY;
        TDM_ERR("'private_display != NULL' failed");
        goto failed_alloc;
    }

    if (pthread_mutex_init(&private_display->lock, NULL))
    {
        ret = TDM_ERROR_OPERATION_FAILED;
        TDM_ERR("mutex init failed: %m");
        goto failed_mutex_init;
    }

    ret = _tdm_display_load_module(private_display);
    if (ret != TDM_ERROR_NONE)
        goto failed_load;

    ret = _tdm_display_update_internal(private_display, 0);
    if (ret != TDM_ERROR_NONE)
        goto failed_update;

    private_display->init_count = 1;

    g_private_display = private_display;

    if (error)
        *error = TDM_ERROR_NONE;

    pthread_mutex_unlock(&gLock);

    return (tdm_display*)private_display;

failed_update:
    _tdm_display_unload_module(private_display);
failed_load:
    pthread_mutex_destroy(&private_display->lock);
failed_mutex_init:
    free(private_display);
failed_alloc:
    tdm_debug = 0;
    if (error)
        *error = ret;
    pthread_mutex_unlock(&gLock);
    return NULL;
}

EXTERN void
tdm_display_deinit(tdm_display *dpy)
{
    tdm_private_display *private_display = dpy;

    if (!private_display)
        return;

    pthread_mutex_lock(&gLock);

    private_display->init_count--;
    if (private_display->init_count > 0)
    {
        pthread_mutex_unlock(&gLock);
        return;
    }

    pthread_mutex_lock(&private_display->lock);

    _tdm_display_destroy_private_display(private_display);
    _tdm_display_unload_module(private_display);

    pthread_mutex_unlock(&private_display->lock);

    pthread_mutex_destroy(&private_display->lock);
    free(private_display);
    tdm_debug = 0;

    pthread_mutex_unlock(&gLock);
}

