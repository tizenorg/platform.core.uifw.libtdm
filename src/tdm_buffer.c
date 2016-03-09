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

typedef struct _tdm_buffer_info {
	tbm_surface_h buffer;

	/* ref_count for backend */
	int backend_ref_count;

	struct list_head release_funcs;
	struct list_head destroy_funcs;

	struct list_head link;
} tdm_buffer_info;

static void
_tdm_buffer_destroy_info(void *user_data)
{
	tdm_buffer_info *buf_info = (tdm_buffer_info *)user_data;
	tdm_buffer_func_info *func_info = NULL, *next = NULL;

	if (buf_info->backend_ref_count > 0)
		TDM_NEVER_GET_HERE();

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

	LIST_DEL(&buf_info->link);

	free(buf_info);
}

static tdm_buffer_info *
_tdm_buffer_get_info(tbm_surface_h buffer)
{
	tdm_buffer_info *buf_info = NULL;
	tbm_bo bo;

	bo = tbm_surface_internal_get_bo(buffer, 0);
	TDM_RETURN_VAL_IF_FAIL(bo != NULL, NULL);

	tbm_bo_get_user_data(bo, TDM_BUFFER_KEY, (void **)&buf_info);

	if (!buf_info) {
		buf_info = calloc(1, sizeof(tdm_buffer_info));
		TDM_RETURN_VAL_IF_FAIL(buf_info != NULL, NULL);

		buf_info->buffer = buffer;

		LIST_INITHEAD(&buf_info->release_funcs);
		LIST_INITHEAD(&buf_info->destroy_funcs);
		LIST_INITHEAD(&buf_info->link);

		tbm_bo_add_user_data(bo, TDM_BUFFER_KEY, _tdm_buffer_destroy_info);
		tbm_bo_set_user_data(bo, TDM_BUFFER_KEY, buf_info);
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

	buf_info = _tdm_buffer_get_info(buffer);
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

	buf_info = _tdm_buffer_get_info(buffer);
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

	buf_info = _tdm_buffer_get_info(buffer);
	TDM_RETURN_VAL_IF_FAIL(buf_info != NULL, NULL);

	buf_info->backend_ref_count++;

	return buffer;
}

EXTERN void
tdm_buffer_unref_backend(tbm_surface_h buffer)
{
	tdm_buffer_info *buf_info;
	tdm_buffer_func_info *func_info = NULL, *next = NULL;

	TDM_RETURN_IF_FAIL(buffer != NULL);

	buf_info = _tdm_buffer_get_info(buffer);
	TDM_RETURN_IF_FAIL(buf_info != NULL);

	buf_info->backend_ref_count--;

	if (buf_info->backend_ref_count > 0)
		return;

	LIST_FOR_EACH_ENTRY_SAFE(func_info, next, &buf_info->release_funcs, link) {
		tbm_surface_internal_ref(buffer);
		func_info->release_func(buffer, func_info->user_data);
		tbm_surface_internal_unref(buffer);
	}
}

EXTERN tdm_error
tdm_buffer_add_destroy_handler(tbm_surface_h buffer,
                               tdm_buffer_destroy_handler func, void *user_data)
{
	tdm_buffer_info *buf_info;
	tdm_buffer_func_info *func_info;

	TDM_RETURN_VAL_IF_FAIL(buffer != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(func != NULL, TDM_ERROR_INVALID_PARAMETER);

	buf_info = _tdm_buffer_get_info(buffer);
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

	buf_info = _tdm_buffer_get_info(buffer);
	TDM_RETURN_IF_FAIL(buf_info != NULL);

	LIST_FOR_EACH_ENTRY_SAFE(func_info, next, &buf_info->destroy_funcs, link) {
		if (func_info->destroy_func != func || func_info->user_data != user_data)
			continue;

		LIST_DEL(&func_info->link);
		free(func_info);

		return;
	}
}

INTERN void
tdm_buffer_add_list(struct list_head *list, tbm_surface_h buffer)
{
	tdm_buffer_info *buf_info;

	TDM_RETURN_IF_FAIL(list != NULL);
	TDM_RETURN_IF_FAIL(buffer != NULL);

	buf_info = _tdm_buffer_get_info(buffer);
	TDM_RETURN_IF_FAIL(buf_info != NULL);

	if (buf_info->link.prev != buf_info->link.next) {
		TDM_ERR("%p already added other list\n", buffer);
		return;
	}

	LIST_ADD(&buf_info->link, list);
}

INTERN void
tdm_buffer_remove_list(struct list_head *list, tbm_surface_h buffer)
{
	tdm_buffer_info *buf_info, *b = NULL, *bb = NULL;

	TDM_RETURN_IF_FAIL(list != NULL);

	if (!buffer)
		return;

	buf_info = _tdm_buffer_get_info(buffer);
	TDM_RETURN_IF_FAIL(buf_info != NULL);

	LIST_FOR_EACH_ENTRY_SAFE(b, bb, list, link) {
		if (b == buf_info) {
			LIST_DEL(&buf_info->link);
			return;
		}
	}
}

INTERN void
tdm_buffer_dump_list(struct list_head *list, char *str, int len)
{
	tdm_buffer_info *buf_info = NULL;

	TDM_RETURN_IF_FAIL(list != NULL);

	LIST_FOR_EACH_ENTRY(buf_info, list, link) {
		if (len > 0) {
			int l = snprintf(str, len, " %p", buf_info->buffer);
			str += l;
			len -= l;
		}
		else
			break;
	}
}
