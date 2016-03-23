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

#define LAYER_FUNC_ENTRY_VOID_RETURN() \
    tdm_private_display *private_display; \
    tdm_private_output *private_output; \
    tdm_private_layer *private_layer; \
    tdm_error ret = TDM_ERROR_NONE; /* default TDM_ERROR_NONE */\
    TDM_RETURN_IF_FAIL(layer != NULL); \
    private_layer = (tdm_private_layer*)layer; \
    private_output = private_layer->private_output; \
    private_display = private_output->private_display

EXTERN tdm_error
tdm_display_get_capabilities(tdm_display *dpy,
                             tdm_display_capability *capabilities)
{
	DISPLAY_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(capabilities != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	*capabilities = private_display->capabilities;

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_display_get_pp_capabilities(tdm_display *dpy,
                                tdm_pp_capability *capabilities)
{
	DISPLAY_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(capabilities != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	if (!(private_display->capabilities & TDM_DISPLAY_CAPABILITY_PP)) {
		TDM_ERR("no pp capability");
		_pthread_mutex_unlock(&private_display->lock);
		return TDM_ERROR_NO_CAPABILITY;
	}

	*capabilities = private_display->caps_pp.capabilities;

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_display_get_pp_available_formats(tdm_display *dpy,
                                     const tbm_format **formats, int *count)
{
	DISPLAY_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(formats != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(count != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	if (!(private_display->capabilities & TDM_DISPLAY_CAPABILITY_PP)) {
		TDM_ERR("no pp capability");
		_pthread_mutex_unlock(&private_display->lock);
		return TDM_ERROR_NO_CAPABILITY;
	}

	*formats = (const tbm_format *)private_display->caps_pp.formats;
	*count = private_display->caps_pp.format_count;

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_display_get_pp_available_size(tdm_display *dpy, int *min_w, int *min_h,
                                  int *max_w, int *max_h, int *preferred_align)
{
	DISPLAY_FUNC_ENTRY();

	_pthread_mutex_lock(&private_display->lock);

	if (!(private_display->capabilities & TDM_DISPLAY_CAPABILITY_PP)) {
		TDM_ERR("no pp capability");
		_pthread_mutex_unlock(&private_display->lock);
		return TDM_ERROR_NO_CAPABILITY;
	}

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

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_display_get_capture_capabilities(tdm_display *dpy,
                                     tdm_capture_capability *capabilities)
{
	DISPLAY_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(capabilities != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	if (!(private_display->capabilities & TDM_DISPLAY_CAPABILITY_CAPTURE)) {
		TDM_ERR("no capture capability");
		_pthread_mutex_unlock(&private_display->lock);
		return TDM_ERROR_NO_CAPABILITY;
	}

	*capabilities = private_display->caps_capture.capabilities;

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_display_get_catpure_available_formats(tdm_display *dpy,
                const tbm_format **formats, int *count)
{
	DISPLAY_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(formats != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(count != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	if (!(private_display->capabilities & TDM_DISPLAY_CAPABILITY_CAPTURE)) {
		TDM_ERR("no capture capability");
		_pthread_mutex_unlock(&private_display->lock);
		return TDM_ERROR_NO_CAPABILITY;
	}

	*formats = (const tbm_format *)private_display->caps_capture.formats;
	*count = private_display->caps_capture.format_count;

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_display_get_output_count(tdm_display *dpy, int *count)
{
	tdm_private_output *private_output = NULL;

	DISPLAY_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(count != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	*count = 0;
	LIST_FOR_EACH_ENTRY(private_output, &private_display->output_list, link)
	(*count)++;

	if (*count == 0) {
		_pthread_mutex_unlock(&private_display->lock);
		return TDM_ERROR_NONE;
	}

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}


EXTERN tdm_output *
tdm_display_get_output(tdm_display *dpy, int index, tdm_error *error)
{
	tdm_private_output *private_output = NULL;
	int i = 0;

	DISPLAY_FUNC_ENTRY_ERROR();

	_pthread_mutex_lock(&private_display->lock);

	if (error)
		*error = TDM_ERROR_NONE;

	i = 0;
	LIST_FOR_EACH_ENTRY(private_output, &private_display->output_list, link) {
		if (i == index) {
			_pthread_mutex_unlock(&private_display->lock);
			return private_output;
		}
		i++;
	}

	_pthread_mutex_unlock(&private_display->lock);

	return NULL;
}

EXTERN tdm_error
tdm_display_get_fd(tdm_display *dpy, int *fd)
{
	tdm_func_display *func_display;
	DISPLAY_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(fd != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	func_display = &private_display->func_display;

	if (!func_display->display_get_fd) {
		pthread_mutex_unlock(&private_display->lock);
		return TDM_ERROR_NONE;
	}

	ret = func_display->display_get_fd(private_display->bdata, fd);

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_display_handle_events(tdm_display *dpy)
{
	tdm_func_display *func_display;
	DISPLAY_FUNC_ENTRY();

	_pthread_mutex_lock(&private_display->lock);

	func_display = &private_display->func_display;

	if (!func_display->display_handle_events) {
		pthread_mutex_unlock(&private_display->lock);
		return TDM_ERROR_NONE;
	}

	ret = func_display->display_handle_events(private_display->bdata);

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_pp *
tdm_display_create_pp(tdm_display *dpy, tdm_error *error)
{
	tdm_pp *pp;

	DISPLAY_FUNC_ENTRY_ERROR();

	_pthread_mutex_lock(&private_display->lock);

	pp = (tdm_pp *)tdm_pp_create_internal(private_display, error);

	_pthread_mutex_unlock(&private_display->lock);

	return pp;
}

EXTERN tdm_error
tdm_output_get_model_info(tdm_output *output, const char **maker,
                          const char **model, const char **name)
{
	OUTPUT_FUNC_ENTRY();

	_pthread_mutex_lock(&private_display->lock);

	if (maker)
		*maker = private_output->caps.maker;
	if (model)
		*model = private_output->caps.model;
	if (name)
		*name = private_output->caps.name;

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_output_get_conn_status(tdm_output *output, tdm_output_conn_status *status)
{
	OUTPUT_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(status != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	*status = private_output->caps.status;

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_output_get_output_type(tdm_output *output, tdm_output_type *type)
{
	OUTPUT_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(type != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	*type = private_output->caps.type;

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_output_get_layer_count(tdm_output *output, int *count)
{
	tdm_private_layer *private_layer = NULL;

	OUTPUT_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(count != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	*count = 0;
	LIST_FOR_EACH_ENTRY(private_layer, &private_output->layer_list, link)
	(*count)++;
	if (*count == 0) {
		_pthread_mutex_unlock(&private_display->lock);
		return TDM_ERROR_NONE;
	}

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}


EXTERN tdm_layer *
tdm_output_get_layer(tdm_output *output, int index, tdm_error *error)
{
	tdm_private_layer *private_layer = NULL;
	int i = 0;

	OUTPUT_FUNC_ENTRY_ERROR();

	_pthread_mutex_lock(&private_display->lock);

	if (error)
		*error = TDM_ERROR_NONE;

	LIST_FOR_EACH_ENTRY(private_layer, &private_output->layer_list, link) {
		if (i == index) {
			_pthread_mutex_unlock(&private_display->lock);
			return private_layer;
		}
		i++;
	}

	_pthread_mutex_unlock(&private_display->lock);

	return NULL;
}

EXTERN tdm_error
tdm_output_get_available_properties(tdm_output *output, const tdm_prop **props,
                                    int *count)
{
	OUTPUT_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(props != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(count != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	*props = (const tdm_prop *)private_output->caps.props;
	*count = private_output->caps.prop_count;

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_output_get_available_modes(tdm_output *output,
                               const tdm_output_mode **modes, int *count)
{
	OUTPUT_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(modes != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(count != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	*modes = (const tdm_output_mode *)private_output->caps.modes;
	*count = private_output->caps.mode_count;

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_output_get_available_size(tdm_output *output, int *min_w, int *min_h,
                              int *max_w, int *max_h, int *preferred_align)
{
	OUTPUT_FUNC_ENTRY();

	_pthread_mutex_lock(&private_display->lock);

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

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_output_get_physical_size(tdm_output *output, unsigned int *mmWidth,
                             unsigned int *mmHeight)
{
	OUTPUT_FUNC_ENTRY();

	_pthread_mutex_lock(&private_display->lock);

	if (mmWidth)
		*mmWidth = private_output->caps.mmWidth;
	if (mmHeight)
		*mmHeight = private_output->caps.mmHeight;

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_output_get_subpixel(tdm_output *output, unsigned int *subpixel)
{
	OUTPUT_FUNC_ENTRY();
	TDM_RETURN_VAL_IF_FAIL(subpixel != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	*subpixel = private_output->caps.subpixel;

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_output_get_pipe(tdm_output *output, unsigned int *pipe)
{
	OUTPUT_FUNC_ENTRY();
	TDM_RETURN_VAL_IF_FAIL(pipe != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	*pipe = private_output->pipe;

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}


EXTERN tdm_error
tdm_output_set_property(tdm_output *output, unsigned int id, tdm_value value)
{
	tdm_func_output *func_output;
	OUTPUT_FUNC_ENTRY();

	_pthread_mutex_lock(&private_display->lock);

	func_output = &private_display->func_output;

	if (!func_output->output_set_property) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	ret = func_output->output_set_property(private_output->output_backend, id,
	                                       value);

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_output_get_property(tdm_output *output, unsigned int id, tdm_value *value)
{
	tdm_func_output *func_output;
	OUTPUT_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(value != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	func_output = &private_display->func_output;

	if (!func_output->output_get_property) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	ret = func_output->output_get_property(private_output->output_backend, id,
	                                       value);

	_pthread_mutex_unlock(&private_display->lock);

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

	if (vblank_handler->func) {
		_pthread_mutex_unlock(&private_display->lock);
		vblank_handler->func(vblank_handler->private_output, sequence,
		                     tv_sec, tv_usec, vblank_handler->user_data);
		_pthread_mutex_lock(&private_display->lock);
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
	tdm_private_layer *private_layer = NULL;

	TDM_RETURN_IF_FAIL(commit_handler);

	private_output = commit_handler->private_output;
	private_display = private_output->private_display;

	LIST_FOR_EACH_ENTRY(private_layer, &private_output->layer_list, link) {
		if (!private_layer->waiting_buffer)
			continue;

		if (private_layer->showing_buffer) {
			_pthread_mutex_unlock(&private_display->lock);
			tdm_buffer_unref_backend(private_layer->showing_buffer);
			_pthread_mutex_lock(&private_display->lock);

			if (private_layer->buffer_queue) {
				_pthread_mutex_unlock(&private_display->lock);
				tbm_surface_queue_release(private_layer->buffer_queue,
				                          private_layer->showing_buffer);
				_pthread_mutex_lock(&private_display->lock);
			}
		}

		private_layer->showing_buffer = private_layer->waiting_buffer;
		private_layer->waiting_buffer = NULL;

		if (tdm_debug_buffer)
			TDM_INFO("layer(%p) waiting_buffer(%p) showing_buffer(%p)",
			         private_layer, private_layer->waiting_buffer,
			         private_layer->showing_buffer);
	}

	if (commit_handler->func) {
		_pthread_mutex_unlock(&private_display->lock);
		commit_handler->func(private_output, sequence,
		                     tv_sec, tv_usec, commit_handler->user_data);
		_pthread_mutex_lock(&private_display->lock);
	}

	LIST_DEL(&commit_handler->link);
	free(commit_handler);
}

EXTERN tdm_error
tdm_output_wait_vblank(tdm_output *output, int interval, int sync,
                       tdm_output_vblank_handler func, void *user_data)
{
	tdm_func_output *func_output;
	tdm_private_vblank_handler *vblank_handler;
	OUTPUT_FUNC_ENTRY();

	_pthread_mutex_lock(&private_display->lock);

	func_output = &private_display->func_output;

	if (!func_output->output_wait_vblank) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	vblank_handler = calloc(1, sizeof(tdm_private_vblank_handler));
	if (!vblank_handler) {
		TDM_ERR("failed: alloc memory");
		_pthread_mutex_unlock(&private_display->lock);
		return TDM_ERROR_OUT_OF_MEMORY;
	}

	LIST_ADD(&vblank_handler->link, &private_output->vblank_handler_list);
	vblank_handler->private_output = private_output;
	vblank_handler->func = func;
	vblank_handler->user_data = user_data;

	ret = func_output->output_wait_vblank(private_output->output_backend, interval,
	                                      sync, vblank_handler);
	if (ret != TDM_ERROR_NONE) {
		_pthread_mutex_unlock(&private_display->lock);
		return ret;
	}

	if (!private_output->regist_vblank_cb) {
		private_output->regist_vblank_cb = 1;
		ret = func_output->output_set_vblank_handler(private_output->output_backend,
		                _tdm_output_cb_vblank);
	}

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

static tdm_error
_tdm_output_commit(tdm_output *output, int sync, tdm_output_commit_handler func,
                   void *user_data)
{
	tdm_func_output *func_output;
	tdm_private_commit_handler *commit_handler;
	OUTPUT_FUNC_ENTRY();

	func_output = &private_display->func_output;

	if (!func_output->output_commit) {
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	commit_handler = calloc(1, sizeof(tdm_private_commit_handler));
	if (!commit_handler) {
		TDM_ERR("failed: alloc memory");
		return TDM_ERROR_OUT_OF_MEMORY;
	}

	LIST_ADD(&commit_handler->link, &private_output->commit_handler_list);
	commit_handler->private_output = private_output;
	commit_handler->func = func;
	commit_handler->user_data = user_data;

	ret = func_output->output_commit(private_output->output_backend, sync,
	                                 commit_handler);
	TDM_RETURN_VAL_IF_FAIL(ret == TDM_ERROR_NONE, ret);

	if (!private_output->regist_commit_cb) {
		private_output->regist_commit_cb = 1;
		ret = func_output->output_set_commit_handler(private_output->output_backend,
		                _tdm_output_cb_commit);
	}

	return ret;
}

EXTERN tdm_error
tdm_output_commit(tdm_output *output, int sync, tdm_output_commit_handler func,
                  void *user_data)
{
	OUTPUT_FUNC_ENTRY();

	_pthread_mutex_lock(&private_display->lock);

	ret = _tdm_output_commit(output, sync, func, user_data);

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_output_set_mode(tdm_output *output, const tdm_output_mode *mode)
{
	tdm_func_output *func_output;
	OUTPUT_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(mode != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	func_output = &private_display->func_output;

	if (!func_output->output_set_mode) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	ret = func_output->output_set_mode(private_output->output_backend, mode);

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_output_get_mode(tdm_output *output, const tdm_output_mode **mode)
{
	tdm_func_output *func_output;
	OUTPUT_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(mode != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	func_output = &private_display->func_output;

	if (!func_output->output_get_mode) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	ret = func_output->output_get_mode(private_output->output_backend, mode);

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_output_set_dpms(tdm_output *output, tdm_output_dpms dpms_value)
{
	tdm_func_output *func_output;
	OUTPUT_FUNC_ENTRY();

	if (dpms_value < TDM_OUTPUT_DPMS_ON)
		dpms_value = TDM_OUTPUT_DPMS_ON;
	else if (dpms_value > TDM_OUTPUT_DPMS_OFF)
		dpms_value = TDM_OUTPUT_DPMS_OFF;

	_pthread_mutex_lock(&private_display->lock);

	func_output = &private_display->func_output;

	if (!func_output->output_set_dpms) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	ret = func_output->output_set_dpms(private_output->output_backend, dpms_value);

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_output_get_dpms(tdm_output *output, tdm_output_dpms *dpms_value)
{
	tdm_func_output *func_output;
	OUTPUT_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(dpms_value != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	func_output = &private_display->func_output;

	if (!func_output->output_get_dpms) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	ret = func_output->output_get_dpms(private_output->output_backend, dpms_value);

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_capture *
tdm_output_create_capture(tdm_output *output, tdm_error *error)
{
	tdm_capture *capture = NULL;

	OUTPUT_FUNC_ENTRY_ERROR();

	_pthread_mutex_lock(&private_display->lock);

	capture = (tdm_capture *)tdm_capture_create_output_internal(private_output,
	                error);

	_pthread_mutex_unlock(&private_display->lock);

	return capture;
}

EXTERN tdm_error
tdm_layer_get_capabilities(tdm_layer *layer, tdm_layer_capability *capabilities)
{
	LAYER_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(capabilities != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	*capabilities = private_layer->caps.capabilities;

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_layer_get_available_formats(tdm_layer *layer, const tbm_format **formats,
                                int *count)
{
	LAYER_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(formats != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(count != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	*formats = (const tbm_format *)private_layer->caps.formats;
	*count = private_layer->caps.format_count;

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_layer_get_available_properties(tdm_layer *layer, const tdm_prop **props,
                                   int *count)
{
	LAYER_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(props != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(count != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	*props = (const tdm_prop *)private_layer->caps.props;
	*count = private_layer->caps.prop_count;

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_layer_get_zpos(tdm_layer *layer, unsigned int *zpos)
{
	LAYER_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(zpos != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	*zpos = private_layer->caps.zpos;

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_layer_set_property(tdm_layer *layer, unsigned int id, tdm_value value)
{
	tdm_func_layer *func_layer;
	LAYER_FUNC_ENTRY();

	_pthread_mutex_lock(&private_display->lock);

	func_layer = &private_display->func_layer;

	if (!func_layer->layer_set_property) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	ret = func_layer->layer_set_property(private_layer->layer_backend, id, value);

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_layer_get_property(tdm_layer *layer, unsigned int id, tdm_value *value)
{
	tdm_func_layer *func_layer;
	LAYER_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(value != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	func_layer = &private_display->func_layer;

	if (!func_layer->layer_get_property) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	ret = func_layer->layer_get_property(private_layer->layer_backend, id, value);

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_layer_set_info(tdm_layer *layer, tdm_info_layer *info)
{
	tdm_func_layer *func_layer;
	LAYER_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(info != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	func_layer = &private_display->func_layer;

	private_layer->usable = 0;

	if (!func_layer->layer_set_info) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	TDM_INFO("layer(%p) info: src(%dx%d %d,%d %dx%d %c%c%c%c) dst(%d,%d %dx%d) trans(%d)",
	         private_layer, info->src_config.size.h, info->src_config.size.v,
	         info->src_config.pos.x, info->src_config.pos.y,
	         info->src_config.pos.w, info->src_config.pos.h,
	         FOURCC_STR(info->src_config.format),
	         info->dst_pos.x, info->dst_pos.y,
	         info->dst_pos.w, info->dst_pos.h,
	         info->transform);

	ret = func_layer->layer_set_info(private_layer->layer_backend, info);
	TDM_WARNING_IF_FAIL(ret == TDM_ERROR_NONE);

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_layer_get_info(tdm_layer *layer, tdm_info_layer *info)
{
	tdm_func_layer *func_layer;
	LAYER_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(info != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	func_layer = &private_display->func_layer;

	if (!func_layer->layer_get_info) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	ret = func_layer->layer_get_info(private_layer->layer_backend, info);

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_layer_set_buffer(tdm_layer *layer, tbm_surface_h buffer)
{
	tdm_func_layer *func_layer;

	LAYER_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(buffer != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	func_layer = &private_display->func_layer;

	private_layer->usable = 0;

	if (!func_layer->layer_set_buffer) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	ret = func_layer->layer_set_buffer(private_layer->layer_backend, buffer);
	TDM_WARNING_IF_FAIL(ret == TDM_ERROR_NONE);

	if (ret == TDM_ERROR_NONE) {
		/* FIXME: should save to pending_buffer first. And after committing
		 * successfully, need to move to waiting_buffer.
		 */
		if (private_layer->waiting_buffer) {
			_pthread_mutex_unlock(&private_display->lock);
			tdm_buffer_unref_backend(private_layer->waiting_buffer);
			_pthread_mutex_lock(&private_display->lock);
		}

		private_layer->waiting_buffer = tdm_buffer_ref_backend(buffer);
		if (tdm_debug_buffer)
			TDM_INFO("layer(%p) waiting_buffer(%p)",
			         private_layer, private_layer->waiting_buffer);
	}

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_layer_unset_buffer(tdm_layer *layer)
{
	tdm_func_layer *func_layer;
	LAYER_FUNC_ENTRY();

	_pthread_mutex_lock(&private_display->lock);

	func_layer = &private_display->func_layer;

	if (private_layer->waiting_buffer) {
		_pthread_mutex_unlock(&private_display->lock);
		tdm_buffer_unref_backend(private_layer->waiting_buffer);
		_pthread_mutex_lock(&private_display->lock);
		private_layer->waiting_buffer = NULL;

		if (tdm_debug_buffer)
			TDM_INFO("layer(%p) waiting_buffer(%p)",
			         private_layer, private_layer->waiting_buffer);
	}

	if (private_layer->showing_buffer) {
		_pthread_mutex_unlock(&private_display->lock);
		tdm_buffer_unref_backend(private_layer->showing_buffer);
		_pthread_mutex_lock(&private_display->lock);
		private_layer->showing_buffer = NULL;

		if (tdm_debug_buffer)
			TDM_INFO("layer(%p) showing_buffer(%p)",
			         private_layer, private_layer->showing_buffer);
	}

	private_layer->usable = 1;

	if (!func_layer->layer_unset_buffer) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	ret = func_layer->layer_unset_buffer(private_layer->layer_backend);
	TDM_WARNING_IF_FAIL(ret == TDM_ERROR_NONE);

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

static void
_tbm_layer_queue_acquirable_cb(tbm_surface_queue_h surface_queue, void *data)
{
	TDM_RETURN_IF_FAIL(data != NULL);
	tdm_layer *layer = data;
	tdm_func_layer *func_layer;
	tbm_surface_h surface = NULL;
	LAYER_FUNC_ENTRY_VOID_RETURN();

	_pthread_mutex_lock(&private_display->lock);

	func_layer = &private_display->func_layer;
	if (!func_layer->layer_set_buffer) {
		_pthread_mutex_unlock(&private_display->lock);
		return;
	}

	if (TBM_SURFACE_QUEUE_ERROR_NONE != tbm_surface_queue_acquire(
	            private_layer->buffer_queue, &surface) ||
	    surface == NULL) {
		TDM_ERR("layer(%p) tbm_surface_queue_acquire() failed surface:%p",
		        private_layer, surface);
		_pthread_mutex_unlock(&private_display->lock);
		return;
	}

	ret = func_layer->layer_set_buffer(private_layer->layer_backend, surface);
	TDM_WARNING_IF_FAIL(ret == TDM_ERROR_NONE);

	if (ret == TDM_ERROR_NONE) {
		if (private_layer->waiting_buffer) {
			_pthread_mutex_unlock(&private_display->lock);
			tdm_buffer_unref_backend(private_layer->waiting_buffer);
			tbm_surface_queue_release(private_layer->buffer_queue,
			                          private_layer->waiting_buffer);
			_pthread_mutex_lock(&private_display->lock);
		}

		private_layer->waiting_buffer = tdm_buffer_ref_backend(surface);

		if (tdm_debug_buffer)
			TDM_INFO("layer(%p) waiting_buffer(%p)",
			         private_layer, private_layer->waiting_buffer);

		ret = _tdm_output_commit(private_layer->private_output, 0, NULL, NULL);
		if (ret != TDM_ERROR_NONE)
			TDM_ERR("layer(%p) _tdm_output_commit() is fail", private_layer);
	}

	_pthread_mutex_unlock(&private_display->lock);
}

static void
_tbm_layer_queue_destroy_cb(tbm_surface_queue_h surface_queue, void *data)
{
	TDM_RETURN_IF_FAIL(data != NULL);
	tdm_layer *layer = data;
	LAYER_FUNC_ENTRY_VOID_RETURN();
	TDM_RETURN_IF_FAIL(ret == TDM_ERROR_NONE);

	_pthread_mutex_lock(&private_display->lock);

	if (private_layer->waiting_buffer) {
		_pthread_mutex_unlock(&private_display->lock);
		tdm_buffer_unref_backend(private_layer->waiting_buffer);
		tbm_surface_queue_release(private_layer->buffer_queue,
		                          private_layer->waiting_buffer);
		_pthread_mutex_lock(&private_display->lock);
	}

	private_layer->buffer_queue = NULL;

	_pthread_mutex_unlock(&private_display->lock);
}

EXTERN tdm_error
tdm_layer_set_buffer_queue(tdm_layer *layer, tbm_surface_queue_h buffer_queue)
{
	tdm_func_layer *func_layer;
	LAYER_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(buffer_queue != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	func_layer = &private_display->func_layer;

	private_layer->usable = 0;

	if (!func_layer->layer_set_buffer) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	if (buffer_queue == private_layer->buffer_queue) {
		_pthread_mutex_unlock(&private_display->lock);
		return TDM_ERROR_NONE;
	}

	if (private_layer->waiting_buffer) {
		_pthread_mutex_unlock(&private_display->lock);
		tdm_buffer_unref_backend(private_layer->waiting_buffer);
		tbm_surface_queue_release(private_layer->buffer_queue,
		                          private_layer->waiting_buffer);
		private_layer->waiting_buffer = NULL;
		_pthread_mutex_lock(&private_display->lock);

		if (tdm_debug_buffer)
			TDM_INFO("layer(%p) waiting_buffer(%p)",
			         private_layer, private_layer->waiting_buffer);
	}

	private_layer->buffer_queue = buffer_queue;
	tbm_surface_queue_add_acquirable_cb(private_layer->buffer_queue,
	                                    _tbm_layer_queue_acquirable_cb,
	                                    layer);
	tbm_surface_queue_add_destroy_cb(private_layer->buffer_queue,
	                                 _tbm_layer_queue_destroy_cb,
	                                 layer);
	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_layer_unset_buffer_queue(tdm_layer *layer)
{
	tdm_func_layer *func_layer;
	LAYER_FUNC_ENTRY();

	_pthread_mutex_lock(&private_display->lock);

	func_layer = &private_display->func_layer;

	if (private_layer->waiting_buffer) {
		_pthread_mutex_unlock(&private_display->lock);
		tdm_buffer_unref_backend(private_layer->waiting_buffer);
		tbm_surface_queue_release(private_layer->buffer_queue,
		                          private_layer->waiting_buffer);
		private_layer->waiting_buffer = NULL;
		_pthread_mutex_lock(&private_display->lock);

		if (tdm_debug_buffer)
			TDM_INFO("layer(%p) waiting_buffer(%p)",
			         private_layer, private_layer->waiting_buffer);
	}

	if (private_layer->showing_buffer) {
		_pthread_mutex_unlock(&private_display->lock);
		tdm_buffer_unref_backend(private_layer->showing_buffer);
		tbm_surface_queue_release(private_layer->buffer_queue,
		                          private_layer->showing_buffer);
		_pthread_mutex_lock(&private_display->lock);
		private_layer->showing_buffer = NULL;

		if (tdm_debug_buffer)
			TDM_INFO("layer(%p) showing_buffer(%p)",
			         private_layer, private_layer->showing_buffer);
	}

	tbm_surface_queue_remove_acquirable_cb(private_layer->buffer_queue, _tbm_layer_queue_acquirable_cb, layer);
	tbm_surface_queue_remove_destroy_cb(private_layer->buffer_queue, _tbm_layer_queue_destroy_cb, layer);
	private_layer->buffer_queue = NULL;
	private_layer->usable = 1;

	if (!func_layer->layer_unset_buffer) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	ret = func_layer->layer_unset_buffer(private_layer->layer_backend);

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_layer_is_usable(tdm_layer *layer, unsigned int *usable)
{
	LAYER_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(usable != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	*usable = private_layer->usable;

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_layer_set_video_pos(tdm_layer *layer, int zpos)
{
	tdm_func_layer *func_layer;
	LAYER_FUNC_ENTRY();

	_pthread_mutex_lock(&private_display->lock);

	func_layer = &private_display->func_layer;

	if (!(private_layer->caps.capabilities & TDM_LAYER_CAPABILITY_VIDEO)) {
		TDM_ERR("layer(%p) is not video layer", private_layer);
		_pthread_mutex_unlock(&private_display->lock);
		return TDM_ERROR_INVALID_PARAMETER;
	}

	if (!func_layer->layer_set_video_pos) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	ret = func_layer->layer_set_video_pos(private_layer->layer_backend, zpos);

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_capture *
tdm_layer_create_capture(tdm_layer *layer, tdm_error *error)
{
	tdm_capture *capture = NULL;

	LAYER_FUNC_ENTRY_ERROR();

	_pthread_mutex_lock(&private_display->lock);

	capture = (tdm_capture *)tdm_capture_create_layer_internal(private_layer,
	                error);

	_pthread_mutex_unlock(&private_display->lock);

	return capture;
}
