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

typedef struct _tdm_buffer_func_info
{
    tdm_buffer_release_handler func;
    void *user_data;

    struct list_head link;
} tdm_buffer_func_info;

typedef struct _tdm_buffer_info
{
    tbm_surface_h buffer;

    /* frontend ref_count */
    int ref_count;

    /* backend ref_count */
    int backend_ref_count;

    struct list_head release_funcs;
    struct list_head link;
} tdm_buffer_info;

static int buffer_list_init;
static struct list_head buffer_list;

EXTERN tdm_buffer*
tdm_buffer_create(tbm_surface_h buffer, tdm_error *error)
{
    tdm_buffer_info *buf_info;

    if (!buffer_list_init)
    {
        LIST_INITHEAD(&buffer_list);
        buffer_list_init = 1;
    }

    if (!buffer)
    {
        if (error)
            *error = TDM_ERROR_INVALID_PARAMETER;

        TDM_ERR("'buffer != NULL' failed");

        return NULL;
    }

    buf_info = calloc(1, sizeof(tdm_buffer_info));
    if (!buf_info)
    {
        if (error)
            *error = TDM_ERROR_OUT_OF_MEMORY;

        TDM_ERR("'buf_info != NULL' failed");

        return NULL;
    }

    buf_info->ref_count = 1;

    tbm_surface_internal_ref(buffer);
    buf_info->buffer = buffer;

    LIST_INITHEAD(&buf_info->release_funcs);
    LIST_ADDTAIL(&buf_info->link, &buffer_list);

    if (error)
        *error = TDM_ERROR_NONE;

    return (tdm_buffer*)buf_info;
}

EXTERN tdm_buffer*
tdm_buffer_ref(tdm_buffer *buffer, tdm_error *error)
{
    tdm_buffer_info *buf_info;

    if (!buffer)
    {
        if (error)
            *error = TDM_ERROR_INVALID_PARAMETER;

        TDM_ERR("'buffer != NULL' failed");

        return NULL;
    }

    buf_info = buffer;
    buf_info->ref_count++;

    if (error)
        *error = TDM_ERROR_NONE;

    return buffer;
}

EXTERN void
tdm_buffer_unref(tdm_buffer *buffer)
{
    tdm_buffer_info *buf_info;
    tdm_buffer_func_info *func_info = NULL, *next = NULL;

    if (!buffer)
        return;

    buf_info = buffer;
    buf_info->ref_count--;

    if (buf_info->ref_count > 0)
        return;

    /* Before ref_count become 0, all backend reference should be removed */
    TDM_WARNING_IF_FAIL(buf_info->backend_ref_count == 0);

    LIST_FOR_EACH_ENTRY_SAFE(func_info, next, &buf_info->release_funcs, link)
    {
        LIST_DEL(&func_info->link);
        free(func_info);
    }

    LIST_DEL(&buf_info->link);

    tbm_surface_internal_unref(buf_info->buffer);

    free(buf_info);
}

EXTERN tdm_error
tdm_buffer_add_release_handler(tdm_buffer *buffer,
                               tdm_buffer_release_handler func, void *user_data)
{
    tdm_buffer_info *buf_info;
    tdm_buffer_func_info *func_info;

    TDM_RETURN_VAL_IF_FAIL(buffer != NULL, TDM_ERROR_INVALID_PARAMETER);
    TDM_RETURN_VAL_IF_FAIL(func != NULL, TDM_ERROR_INVALID_PARAMETER);

    func_info = calloc(1, sizeof(tdm_buffer_func_info));
    TDM_RETURN_VAL_IF_FAIL(func_info != NULL, TDM_ERROR_OUT_OF_MEMORY);

    func_info->func = func;
    func_info->user_data = user_data;

    buf_info = buffer;
    LIST_ADD(&func_info->link, &buf_info->release_funcs);

    return TDM_ERROR_NONE;
}

EXTERN void
tdm_buffer_remove_release_handler(tdm_buffer *buffer, tdm_buffer_release_handler func, void *user_data)
{
    tdm_buffer_info *buf_info;
    tdm_buffer_func_info *func_info = NULL, *next = NULL;

    TDM_RETURN_IF_FAIL(buffer != NULL);
    TDM_RETURN_IF_FAIL(func != NULL);

    buf_info = buffer;
    LIST_FOR_EACH_ENTRY_SAFE(func_info, next, &buf_info->release_funcs, link)
    {
        if (func_info->func != func || func_info->user_data != user_data)
            continue;

        LIST_DEL(&func_info->link);
        free(func_info);

        return;
    }
}


INTERN tdm_buffer*
tdm_buffer_ref_backend(tdm_buffer *buffer)
{
    tdm_buffer_info *buf_info;

    TDM_RETURN_VAL_IF_FAIL(buffer != NULL, NULL);

    buf_info = buffer;
    buf_info->backend_ref_count++;

    return buffer;
}

INTERN void
tdm_buffer_unref_backend(tdm_buffer *buffer)
{
    tdm_buffer_info *buf_info;
    tdm_buffer_func_info *func_info = NULL, *next = NULL;

    TDM_RETURN_IF_FAIL(buffer != NULL);

    buf_info = buffer;
    buf_info->backend_ref_count--;

    if (buf_info->backend_ref_count > 0)
        return;

    LIST_FOR_EACH_ENTRY_SAFE(func_info, next, &buf_info->release_funcs, link)
        func_info->func(buffer, func_info->user_data);
}

INTERN tbm_surface_h
tdm_buffer_get_surface(tdm_buffer *buffer)
{
    tdm_buffer_info *buf_info = buffer;

    TDM_RETURN_VAL_IF_FAIL(buf_info != NULL, NULL);

    return buf_info->buffer;
}

INTERN tdm_buffer*
tdm_buffer_get(tbm_surface_h buffer)
{
    tdm_buffer_info *found;

    TDM_RETURN_VAL_IF_FAIL(buffer != NULL, NULL);

    if (!buffer_list_init)
    {
        LIST_INITHEAD(&buffer_list);
        buffer_list_init = 1;
    }

    LIST_FOR_EACH_ENTRY(found, &buffer_list, link)
    {
        if (found->buffer == buffer)
            return found;
    }

    return NULL;
}
