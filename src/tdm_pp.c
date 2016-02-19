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

#define PP_FUNC_ENTRY() \
    tdm_func_pp *func_pp; \
    tdm_private_display *private_display; \
    tdm_private_pp *private_pp; \
    tdm_error ret = TDM_ERROR_NONE; \
    TDM_RETURN_VAL_IF_FAIL(pp != NULL, TDM_ERROR_INVALID_PARAMETER); \
    private_pp = (tdm_private_pp*)pp; \
    private_display = private_pp->private_display; \
    func_pp = private_pp->func_pp

static void
_tdm_pp_cb_done(tdm_pp *pp_backend, tbm_surface_h src, tbm_surface_h dst,
                void *user_data)
{
	tdm_private_pp *private_pp = user_data;
	tdm_private_display *private_display = private_pp->private_display;
	int lock_after_cb_done = 0;
	int ret;

	ret = pthread_mutex_trylock(&private_display->lock);
	if (ret == 0)
		pthread_mutex_unlock(&private_display->lock);
	else  if (ret == EBUSY) {
		pthread_mutex_unlock(&private_display->lock);
		lock_after_cb_done = 1;
	}

	tdm_buffer_unref_backend(src);
	tdm_buffer_unref_backend(dst);

	if (lock_after_cb_done)
		pthread_mutex_lock(&private_display->lock);
}

INTERN tdm_private_pp *
tdm_pp_create_internal(tdm_private_display *private_display, tdm_error *error)
{
	tdm_func_display *func_display;
	tdm_func_pp *func_pp;
	tdm_private_pp *private_pp = NULL;
	tdm_pp *pp_backend = NULL;
	tdm_error ret = TDM_ERROR_NONE;

	func_display = &private_display->func_display;
	func_pp = &private_display->func_pp;

	if (!(private_display->capabilities & TDM_DISPLAY_CAPABILITY_PP)) {
		TDM_ERR("no pp capability");
		if (error)
			*error = TDM_ERROR_NO_CAPABILITY;
		return NULL;
	}

	pp_backend = func_display->display_create_pp(private_display->bdata, &ret);
	if (ret != TDM_ERROR_NONE) {
		if (error)
			*error = ret;
		return NULL;
	}

	private_pp = calloc(1, sizeof(tdm_private_pp));
	if (!private_pp) {
		TDM_ERR("failed: alloc memory");
		func_pp->pp_destroy(pp_backend);
		if (error)
			*error = TDM_ERROR_OUT_OF_MEMORY;
		return NULL;
	}

	ret = func_pp->pp_set_done_handler(pp_backend, _tdm_pp_cb_done, private_pp);
	if (ret != TDM_ERROR_NONE) {
		TDM_ERR("set pp_done_handler failed");
		func_pp->pp_destroy(pp_backend);
		if (error)
			*error = ret;
		return NULL;
	}

	LIST_ADD(&private_pp->link, &private_display->pp_list);
	private_pp->func_pp = func_pp;
	private_pp->private_display = private_display;
	private_pp->pp_backend = pp_backend;

	if (error)
		*error = TDM_ERROR_NONE;

	return private_pp;
}

INTERN void
tdm_pp_destroy_internal(tdm_private_pp *private_pp)
{
	tdm_func_pp *func_pp;

	if (!private_pp)
		return;

	func_pp = private_pp->func_pp;

	LIST_DEL(&private_pp->link);

	func_pp->pp_destroy(private_pp->pp_backend);

	free(private_pp);
}

EXTERN void
tdm_pp_destroy(tdm_pp *pp)
{
	tdm_private_pp *private_pp = pp;
	tdm_private_display *private_display;

	if (!private_pp)
		return;

	private_display = private_pp->private_display;

	pthread_mutex_lock(&private_display->lock);
	tdm_pp_destroy_internal(private_pp);
	pthread_mutex_unlock(&private_display->lock);
}

EXTERN tdm_error
tdm_pp_set_info(tdm_pp *pp, tdm_info_pp *info)
{
	PP_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(info != NULL, TDM_ERROR_INVALID_PARAMETER);

	pthread_mutex_lock(&private_display->lock);

	if (!func_pp->pp_set_info) {
		pthread_mutex_unlock(&private_display->lock);
		return TDM_ERROR_NONE;
	}

	ret = func_pp->pp_set_info(private_pp->pp_backend, info);

	pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_pp_attach(tdm_pp *pp, tbm_surface_h src, tbm_surface_h dst)
{
	PP_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(src != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(dst != NULL, TDM_ERROR_INVALID_PARAMETER);

	pthread_mutex_lock(&private_display->lock);

	if (!func_pp->pp_attach) {
		pthread_mutex_unlock(&private_display->lock);
		return TDM_ERROR_NONE;
	}

	tdm_buffer_ref_backend(src);
	tdm_buffer_ref_backend(dst);
	ret = func_pp->pp_attach(private_pp->pp_backend, src, dst);

	pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_pp_commit(tdm_pp *pp)
{
	PP_FUNC_ENTRY();

	pthread_mutex_lock(&private_display->lock);

	if (!func_pp->pp_commit) {
		pthread_mutex_unlock(&private_display->lock);
		return TDM_ERROR_NONE;
	}

	ret = func_pp->pp_commit(private_pp->pp_backend);

	pthread_mutex_unlock(&private_display->lock);

	return ret;
}
