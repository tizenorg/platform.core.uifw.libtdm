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
#include "tdm_private.h"
#include "tdm_list.h"

static int tdm_buffer_key;
#define TDM_BUFFER_KEY ((unsigned long)&tdm_buffer_key)

typedef struct _tdm_buffer_func_info {
	tdm_buffer_release_handler release_func;
	tdm_buffer_destroy_handler destroy_func;
	void *user_data;

	struct list_head link;
} tdm_buffer_func_info;

static void
_tdm_buffer_destroy_info(void *user_data)
{
	tdm_buffer_info *buf_info = (tdm_buffer_info *)user_data;
	tdm_buffer_func_info *func_info = NULL, *next = NULL;

	if (buf_info->backend_ref_count > 0) {
		TDM_NEVER_GET_HERE();
		if (tdm_debug_buffer)
			TDM_INFO("%p", buf_info->buffer);
	}

	LIST_FOR_EACH_ENTRY_SAFE(func_info, next, &buf_info->release_funcs, link) {
		LIST_DEL(&func_info->link);
		free(func_info);
	}

	LIST_FOR_EACH_ENTRY_SAFE(func_info, next, &buf_info->destroy_funcs, link)
	func_info->destroy_func(buf_info->buffer, func_info->user_data);

	LIST_FOR_EACH_ENTRY_SAFE(func_info, next, &buf_info->destroy_funcs, link) {
		LIST_DEL(&func_info->link);
		free(func_info);
	}

	if (tdm_debug_buffer)
		TDM_INFO("%p destroyed", buf_info->buffer);

	free(buf_info);
}

INTERN tdm_buffer_info *
tdm_buffer_get_info(tbm_surface_h buffer)
{
	tdm_buffer_info *buf_info = NULL;

	if (!tbm_surface_internal_get_user_data(buffer, TDM_BUFFER_KEY, (void **)&buf_info)) {
		buf_info = calloc(1, sizeof(tdm_buffer_info));
		TDM_RETURN_VAL_IF_FAIL(buf_info != NULL, NULL);

		buf_info->buffer = buffer;

		LIST_INITHEAD(&buf_info->release_funcs);
		LIST_INITHEAD(&buf_info->destroy_funcs);
		LIST_INITHEAD(&buf_info->link);

		if (!tbm_surface_internal_add_user_data(buffer, TDM_BUFFER_KEY, _tdm_buffer_destroy_info)) {
			TDM_ERR("FAIL to create user_data for surface %p", buffer);
			return NULL;
		}
		if (!tbm_surface_internal_set_user_data(buffer, TDM_BUFFER_KEY, buf_info)) {
			TDM_ERR("FAIL to set user_data for surface %p", buffer);
			return NULL;
		}

		if (tdm_debug_buffer)
			TDM_INFO("%p created", buf_info->buffer);
	}

	return buf_info;
}

EXTERN tdm_error
tdm_buffer_add_release_handler(tbm_surface_h buffer,
							   tdm_buffer_release_handler func, void *user_data)
{
	tdm_buffer_info *buf_info;
	tdm_buffer_func_info *func_info;

	TDM_RETURN_VAL_IF_FAIL(buffer != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(func != NULL, TDM_ERROR_INVALID_PARAMETER);

	buf_info = tdm_buffer_get_info(buffer);
	TDM_RETURN_VAL_IF_FAIL(buf_info != NULL, TDM_ERROR_OUT_OF_MEMORY);

	func_info = calloc(1, sizeof(tdm_buffer_func_info));
	TDM_RETURN_VAL_IF_FAIL(func_info != NULL, TDM_ERROR_OUT_OF_MEMORY);

	func_info->release_func = func;
	func_info->user_data = user_data;

	LIST_ADD(&func_info->link, &buf_info->release_funcs);

	return TDM_ERROR_NONE;
}

EXTERN void
tdm_buffer_remove_release_handler(tbm_surface_h buffer,
								  tdm_buffer_release_handler func, void *user_data)
{
	tdm_buffer_info *buf_info;
	tdm_buffer_func_info *func_info = NULL, *next = NULL;

	TDM_RETURN_IF_FAIL(buffer != NULL);
	TDM_RETURN_IF_FAIL(func != NULL);

	buf_info = tdm_buffer_get_info(buffer);
	TDM_RETURN_IF_FAIL(buf_info != NULL);

	LIST_FOR_EACH_ENTRY_SAFE(func_info, next, &buf_info->release_funcs, link) {
		if (func_info->release_func != func || func_info->user_data != user_data)
			continue;

		LIST_DEL(&func_info->link);
		free(func_info);

		return;
	}
}


EXTERN tbm_surface_h
tdm_buffer_ref_backend(tbm_surface_h buffer)
{
	tdm_buffer_info *buf_info;

	TDM_RETURN_VAL_IF_FAIL(buffer != NULL, NULL);

	buf_info = tdm_buffer_get_info(buffer);
	TDM_RETURN_VAL_IF_FAIL(buf_info != NULL, NULL);

	buf_info->backend_ref_count++;
	tbm_surface_internal_ref(buffer);

	return buffer;
}

EXTERN void
tdm_buffer_unref_backend(tbm_surface_h buffer)
{
	tdm_buffer_info *buf_info;
	tdm_buffer_func_info *func_info = NULL, *next = NULL;

	TDM_RETURN_IF_FAIL(buffer != NULL);

	buf_info = tdm_buffer_get_info(buffer);
	TDM_RETURN_IF_FAIL(buf_info != NULL);

	buf_info->backend_ref_count--;
	if (buf_info->backend_ref_count > 0) {
		tbm_surface_internal_unref(buffer);
		return;
	}

	if (!tdm_thread_in_display_thread(syscall(SYS_gettid)))
		TDM_NEVER_GET_HERE();

	LIST_FOR_EACH_ENTRY_SAFE(func_info, next, &buf_info->release_funcs, link) {
		tbm_surface_internal_ref(buffer);
		func_info->release_func(buffer, func_info->user_data);
		tbm_surface_internal_unref(buffer);
	}

	tbm_surface_internal_unref(buffer);
}

EXTERN tdm_error
tdm_buffer_add_destroy_handler(tbm_surface_h buffer,
							   tdm_buffer_destroy_handler func, void *user_data)
{
	tdm_buffer_info *buf_info;
	tdm_buffer_func_info *func_info;

	TDM_RETURN_VAL_IF_FAIL(buffer != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(func != NULL, TDM_ERROR_INVALID_PARAMETER);

	buf_info = tdm_buffer_get_info(buffer);
	TDM_RETURN_VAL_IF_FAIL(buf_info != NULL, TDM_ERROR_OUT_OF_MEMORY);

	func_info = calloc(1, sizeof(tdm_buffer_func_info));
	TDM_RETURN_VAL_IF_FAIL(func_info != NULL, TDM_ERROR_OUT_OF_MEMORY);

	func_info->destroy_func = func;
	func_info->user_data = user_data;

	LIST_ADD(&func_info->link, &buf_info->destroy_funcs);

	return TDM_ERROR_NONE;
}

EXTERN void
tdm_buffer_remove_destroy_handler(tbm_surface_h buffer,
								  tdm_buffer_destroy_handler func, void *user_data)
{
	tdm_buffer_info *buf_info;
	tdm_buffer_func_info *func_info = NULL, *next = NULL;

	TDM_RETURN_IF_FAIL(buffer != NULL);
	TDM_RETURN_IF_FAIL(func != NULL);

	buf_info = tdm_buffer_get_info(buffer);
	TDM_RETURN_IF_FAIL(buf_info != NULL);

	LIST_FOR_EACH_ENTRY_SAFE(func_info, next, &buf_info->destroy_funcs, link) {
		if (func_info->destroy_func != func || func_info->user_data != user_data)
			continue;

		LIST_DEL(&func_info->link);
		free(func_info);

		return;
	}
}

INTERN tbm_surface_h
tdm_buffer_list_get_first_entry(struct list_head *list)
{
	tdm_buffer_info *buf_info = NULL;

	TDM_RETURN_VAL_IF_FAIL(list != NULL, NULL);

	if (LIST_IS_EMPTY(list))
		return NULL;

	buf_info = container_of((list)->next, buf_info, link);

	return buf_info->buffer;
}

INTERN void
tdm_buffer_list_dump(struct list_head *list)
{
	tdm_buffer_info *buf_info = NULL;
	char str[256], *p;
	int len = sizeof(str);

	TDM_RETURN_IF_FAIL(list != NULL);

	p = str;
	LIST_FOR_EACH_ENTRY(buf_info, list, link) {
		if (len > 0) {
			int l = snprintf(p, len, " %p", buf_info->buffer);
			p += l;
			len -= l;
		} else
			break;
	}

	TDM_INFO("\t %s", str);
}
