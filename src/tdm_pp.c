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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tdm.h"
#include "tdm_backend.h"
#include "tdm_private.h"
#include "tdm_helper.h"

typedef struct _tdm_pp_private_buffer {
	tbm_surface_h src;
	tbm_surface_h dst;
	struct list_head link;
	struct list_head commit_link;
} tdm_pp_private_buffer;

#define PP_FUNC_ENTRY() \
	tdm_func_pp *func_pp; \
	tdm_private_display *private_display; \
	tdm_private_pp *private_pp; \
	tdm_error ret = TDM_ERROR_NONE; \
	TDM_RETURN_VAL_IF_FAIL(pp != NULL, TDM_ERROR_INVALID_PARAMETER); \
	private_pp = (tdm_private_pp*)pp; \
	private_display = private_pp->private_display; \
	func_pp = &private_display->func_pp

static void
_tdm_pp_print_list(struct list_head *list)
{
	tdm_pp_private_buffer *b = NULL;
	char str[512], *p;
	int len = sizeof(str);

	TDM_RETURN_IF_FAIL(list != NULL);

	p = str;
	LIST_FOR_EACH_ENTRY(b, list, link) {
		if (len > 0) {
			int l = snprintf(p, len, " (%p,%p)", b->src, b->dst);
			p += l;
			len -= l;
		} else
			break;
	}

	TDM_INFO("\t %s", str);
}

static tdm_pp_private_buffer *
_tdm_pp_find_tbm_buffers(struct list_head *list, tbm_surface_h src, tbm_surface_h dst)
{
	tdm_pp_private_buffer *b = NULL, *bb = NULL;

	LIST_FOR_EACH_ENTRY_SAFE(b, bb, list, link) {
		if (b->src == src && b->dst == dst)
			return b;
	}

	return NULL;
}

static tdm_pp_private_buffer *
_tdm_pp_find_buffer(struct list_head *list, tdm_pp_private_buffer *pp_buffer)
{
	tdm_pp_private_buffer *b = NULL, *bb = NULL;

	LIST_FOR_EACH_ENTRY_SAFE(b, bb, list, link) {
		if (b == pp_buffer)
			return b;
	}

	return NULL;
}

INTERN void
tdm_pp_cb_done(tdm_pp *pp_backend, tbm_surface_h src, tbm_surface_h dst,
			   void *user_data)
{
	tdm_private_pp *private_pp = user_data;
	tdm_private_display *private_display = private_pp->private_display;
	tdm_pp_private_buffer *pp_buffer = NULL, *first_entry = NULL;

	TDM_RETURN_IF_FAIL(TDM_MUTEX_IS_LOCKED());

	if (private_pp->owner_tid != syscall(SYS_gettid)) {
		tdm_thread_cb_pp_done pp_done;
		tdm_error ret;

		pp_done.base.type = TDM_THREAD_CB_PP_DONE;
		pp_done.base.length = sizeof pp_done;
		pp_done.pp_stamp = private_pp->stamp;
		pp_done.src = src;
		pp_done.dst = dst;
		pp_done.user_data = user_data;

		ret = tdm_thread_send_cb(private_display->private_loop, &pp_done.base);
		TDM_WARNING_IF_FAIL(ret == TDM_ERROR_NONE);

		return;
	}

	if (private_pp->owner_tid != syscall(SYS_gettid))
		TDM_NEVER_GET_HERE();

	if (tdm_debug_dump & TDM_DUMP_FLAG_PP) {
		char str[TDM_PATH_LEN];
		static int i;
		snprintf(str, TDM_PATH_LEN, "pp_dst_%03d", i++);
		tdm_helper_dump_buffer_str(dst, str);
	}

	if (tdm_debug_buffer)
		TDM_INFO("pp(%p) done: src(%p) dst(%p)", private_pp, src, dst);

	if (!LIST_IS_EMPTY(&private_pp->buffer_list)) {
		first_entry = container_of((&private_pp->buffer_list)->next, pp_buffer, link);
		if (first_entry->src != src || first_entry->dst != dst)
			TDM_ERR("buffer(%p,%p) is skipped", first_entry->src, first_entry->dst);
	} else {
		TDM_NEVER_GET_HERE();
	}

	if ((pp_buffer = _tdm_pp_find_tbm_buffers(&private_pp->buffer_list, src, dst))) {
		LIST_DEL(&pp_buffer->link);

		_pthread_mutex_unlock(&private_display->lock);
		tdm_buffer_unref_backend(src);
		tdm_buffer_unref_backend(dst);
		_pthread_mutex_lock(&private_display->lock);

		free(pp_buffer);
	}
}

INTERN tdm_private_pp *
tdm_pp_find_stamp(tdm_private_display *private_display, unsigned long stamp)
{
	tdm_private_pp *private_pp = NULL;

	TDM_RETURN_VAL_IF_FAIL(TDM_MUTEX_IS_LOCKED(), NULL);

	LIST_FOR_EACH_ENTRY(private_pp, &private_display->pp_list, link) {
		if (private_pp->stamp == stamp)
			return private_pp;
	}

	return NULL;
}

INTERN tdm_private_pp *
tdm_pp_create_internal(tdm_private_display *private_display, tdm_error *error)
{
	tdm_func_display *func_display;
	tdm_func_pp *func_pp;
	tdm_private_pp *private_pp = NULL;
	tdm_pp *pp_backend = NULL;
	tdm_error ret = TDM_ERROR_NONE;

	TDM_RETURN_VAL_IF_FAIL(TDM_MUTEX_IS_LOCKED(), NULL);

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

	ret = func_pp->pp_set_done_handler(pp_backend, tdm_pp_cb_done, private_pp);
	if (ret != TDM_ERROR_NONE) {
		TDM_ERR("spp(%p) et pp_done_handler failed", private_pp);
		func_pp->pp_destroy(pp_backend);
		if (error)
			*error = ret;
		return NULL;
	}

	private_pp->stamp = tdm_helper_get_time_in_millis();
	while (tdm_pp_find_stamp(private_display, private_pp->stamp))
		private_pp->stamp++;

	LIST_ADD(&private_pp->link, &private_display->pp_list);
	private_pp->private_display = private_display;
	private_pp->pp_backend = pp_backend;
	private_pp->owner_tid = syscall(SYS_gettid);

	LIST_INITHEAD(&private_pp->pending_buffer_list);
	LIST_INITHEAD(&private_pp->buffer_list);

	if (error)
		*error = TDM_ERROR_NONE;

	return private_pp;
}

INTERN void
tdm_pp_destroy_internal(tdm_private_pp *private_pp)
{
	tdm_private_display *private_display;
	tdm_func_pp *func_pp;
	tdm_pp_private_buffer *b = NULL, *bb = NULL;

	TDM_RETURN_IF_FAIL(TDM_MUTEX_IS_LOCKED());

	if (!private_pp)
		return;

	private_display = private_pp->private_display;
	func_pp = &private_display->func_pp;

	LIST_DEL(&private_pp->link);

	func_pp->pp_destroy(private_pp->pp_backend);

	if (!LIST_IS_EMPTY(&private_pp->pending_buffer_list)) {
		TDM_WRN("pp(%p) not finished:", private_pp);
		_tdm_pp_print_list(&private_pp->pending_buffer_list);

		LIST_FOR_EACH_ENTRY_SAFE(b, bb, &private_pp->pending_buffer_list, link) {
			LIST_DEL(&b->link);
		}
	}

	if (!LIST_IS_EMPTY(&private_pp->buffer_list)) {
		TDM_WRN("pp(%p) not finished:", private_pp);
		_tdm_pp_print_list(&private_pp->buffer_list);

		LIST_FOR_EACH_ENTRY_SAFE(b, bb, &private_pp->buffer_list, link) {
			LIST_DEL(&b->link);
			_pthread_mutex_unlock(&private_display->lock);
			tdm_buffer_unref_backend(b->src);
			tdm_buffer_unref_backend(b->dst);
			_pthread_mutex_lock(&private_display->lock);
		}
	}

	private_pp->stamp = 0;
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

	_pthread_mutex_lock(&private_display->lock);
	tdm_pp_destroy_internal(private_pp);
	_pthread_mutex_unlock(&private_display->lock);
}

EXTERN tdm_error
tdm_pp_set_info(tdm_pp *pp, tdm_info_pp *info)
{
	PP_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(info != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	if (!func_pp->pp_set_info) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	TDM_INFO("pp(%p) info: src(%dx%d %d,%d %dx%d %c%c%c%c) dst(%dx%d %d,%d %dx%d %c%c%c%c) trans(%d) sync(%d) flags(%x)",
			 private_pp, info->src_config.size.h, info->src_config.size.v,
			 info->src_config.pos.x, info->src_config.pos.y,
			 info->src_config.pos.w, info->src_config.pos.h,
			 FOURCC_STR(info->src_config.format),
			 info->dst_config.size.h, info->dst_config.size.v,
			 info->dst_config.pos.x, info->dst_config.pos.y,
			 info->dst_config.pos.w, info->dst_config.pos.h,
			 FOURCC_STR(info->dst_config.format),
			 info->transform, info->sync, info->flags);

	ret = func_pp->pp_set_info(private_pp->pp_backend, info);
	TDM_WARNING_IF_FAIL(ret == TDM_ERROR_NONE);

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_pp_attach(tdm_pp *pp, tbm_surface_h src, tbm_surface_h dst)
{
	tdm_pp_private_buffer *pp_buffer;

	PP_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(src != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(dst != NULL, TDM_ERROR_INVALID_PARAMETER);

	_pthread_mutex_lock(&private_display->lock);

	if (!func_pp->pp_attach) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	if (tdm_display_check_module_abi(private_display, 1, 2) &&
		private_display->caps_pp.max_attach_count > 0) {
		int length = LIST_LENGTH(&private_pp->pending_buffer_list) +
					 LIST_LENGTH(&private_pp->buffer_list);
		if (length >= private_display->caps_pp.max_attach_count) {
			_pthread_mutex_unlock(&private_display->lock);
			TDM_DBG("failed: too many attached!! max_attach_count(%d)",
					private_display->caps_pp.max_attach_count);
			return TDM_ERROR_BAD_REQUEST;
		}
	}

	if (tdm_debug_dump & TDM_DUMP_FLAG_PP) {
		char str[TDM_PATH_LEN];
		static int i;
		snprintf(str, TDM_PATH_LEN, "pp_src_%03d", i++);
		tdm_helper_dump_buffer_str(src, str);
	}

	pp_buffer = calloc(1, sizeof *pp_buffer);
	if (!pp_buffer) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_ERR("alloc failed");
		return TDM_ERROR_OUT_OF_MEMORY;
	}

	ret = func_pp->pp_attach(private_pp->pp_backend, src, dst);
	TDM_WARNING_IF_FAIL(ret == TDM_ERROR_NONE);

	if (ret != TDM_ERROR_NONE) {
		free(pp_buffer);
		_pthread_mutex_unlock(&private_display->lock);
		TDM_ERR("attach failed");
		return ret;
	}

	LIST_ADDTAIL(&pp_buffer->link, &private_pp->pending_buffer_list);
	pp_buffer->src = tdm_buffer_ref_backend(src);
	pp_buffer->dst = tdm_buffer_ref_backend(dst);

	if (tdm_debug_buffer) {
		TDM_INFO("pp(%p) attached:", private_pp);
		_tdm_pp_print_list(&private_pp->pending_buffer_list);
	}

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}

EXTERN tdm_error
tdm_pp_commit(tdm_pp *pp)
{
	tdm_pp_private_buffer *b = NULL, *bb = NULL;
	struct list_head commit_buffer_list;

	PP_FUNC_ENTRY();

	_pthread_mutex_lock(&private_display->lock);

	if (!func_pp->pp_commit) {
		_pthread_mutex_unlock(&private_display->lock);
		TDM_DBG("failed: not implemented!!");
		return TDM_ERROR_NOT_IMPLEMENTED;
	}

	LIST_INITHEAD(&commit_buffer_list);

	LIST_FOR_EACH_ENTRY_SAFE(b, bb, &private_pp->pending_buffer_list, link) {
		LIST_DEL(&b->link);
		LIST_ADDTAIL(&b->link, &private_pp->buffer_list);
		LIST_ADDTAIL(&b->commit_link, &commit_buffer_list);
	}

	ret = func_pp->pp_commit(private_pp->pp_backend);
	TDM_WARNING_IF_FAIL(ret == TDM_ERROR_NONE);

	LIST_FOR_EACH_ENTRY_SAFE(b, bb, &commit_buffer_list, commit_link) {
		if (!_tdm_pp_find_buffer(&private_pp->buffer_list, b))
			continue;

		LIST_DEL(&b->commit_link);

		if (ret != TDM_ERROR_NONE) {
			tdm_buffer_unref_backend(b->src);
			tdm_buffer_unref_backend(b->dst);
			LIST_DEL(&b->link);
		}
	}

	_pthread_mutex_unlock(&private_display->lock);

	return ret;
}
