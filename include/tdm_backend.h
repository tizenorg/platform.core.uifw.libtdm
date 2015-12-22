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

#ifndef _TDM_BACKEND_H_
#define _TDM_BACKEND_H_

#include <tbm_surface.h>

#include "tdm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void tdm_backend_data;

typedef struct _tdm_caps_display
{
    /* -1: not defined */
    int max_layer_count;
} tdm_caps_display;

typedef struct _tdm_caps_output
{
    tdm_output_conn_status status;
    tdm_output_type type;
    unsigned int type_id;

    unsigned int mode_count;
    tdm_output_mode *modes;

    unsigned int prop_count;
    tdm_prop *props;

    unsigned int mmWidth;
    unsigned int mmHeight;
    unsigned int subpixel;

    /* -1: not defined */
    int min_w;
    int min_h;
    int max_w;
    int max_h;
    int preferred_align;
} tdm_caps_output;

typedef struct _tdm_caps_layer
{
    tdm_layer_capability capabilities;
    unsigned int zpos;  /* if VIDEO layer, zpos is -1 */

    unsigned int format_count;
    tbm_format *formats;

    unsigned int prop_count;
    tdm_prop *props;
} tdm_caps_layer;

typedef struct _tdm_caps_pp
{
    tdm_pp_capability capabilities;

    unsigned int format_count;
    tbm_format *formats;

    /* -1: not defined */
    int min_w;
    int min_h;
    int max_w;
    int max_h;
    int preferred_align;
} tdm_caps_pp;

typedef struct _tdm_caps_capture
{
    tdm_capture_capability capabilities;

    unsigned int format_count;
    tbm_format *formats;

    /* -1: not defined */
    int min_w;
    int min_h;
    int max_w;
    int max_h;
    int preferred_align;
} tdm_caps_capture;

typedef struct _tdm_func_display
{
    tdm_error    (*display_get_capabilitiy)(tdm_backend_data *bdata, tdm_caps_display *caps); /* init */
    tdm_error    (*display_get_pp_capability)(tdm_backend_data *bdata, tdm_caps_pp *caps); /* init */
    tdm_error    (*display_get_capture_capability)(tdm_backend_data *bdata, tdm_caps_capture *caps); /* init */
    tdm_output **(*display_get_outputs)(tdm_backend_data *bdata, int *count, tdm_error *error); /* init */
    tdm_error    (*display_get_fd)(tdm_backend_data *bdata, int *fd);
    tdm_error    (*display_handle_events)(tdm_backend_data *bdata);
    tdm_pp*      (*display_create_pp)(tdm_backend_data *bdata, tdm_error *error);

    tdm_error    (*output_get_capability)(tdm_output *output, tdm_caps_output *caps); /* init */
    tdm_layer  **(*output_get_layers)(tdm_output *output, int *count, tdm_error *error); /* init */
    tdm_error    (*output_set_property)(tdm_output *output, unsigned int id, tdm_value value);
    tdm_error    (*output_get_property)(tdm_output *output, unsigned int id, tdm_value *value);
    tdm_error    (*output_wait_vblank)(tdm_output *output, int interval, int sync, void *user_data);
    tdm_error    (*output_set_vblank_handler)(tdm_output *output, tdm_output_vblank_handler func);
    tdm_error    (*output_commit)(tdm_output *output, int sync, void *user_data);
    tdm_error    (*output_set_commit_handler)(tdm_output *output, tdm_output_commit_handler func);
    tdm_error    (*output_set_dpms)(tdm_output *output, tdm_output_dpms dpms_value);
    tdm_error    (*output_get_dpms)(tdm_output *output, tdm_output_dpms *dpms_value);
    tdm_error    (*output_set_mode)(tdm_output *output, const tdm_output_mode *mode);
    tdm_error    (*output_get_mode)(tdm_output *output, const tdm_output_mode **mode);
    tdm_capture *(*output_create_capture)(tdm_output *output, tdm_error *error);

    tdm_error    (*layer_get_capability)(tdm_layer *layer, tdm_caps_layer *caps); /* init */
    tdm_error    (*layer_set_property)(tdm_layer *layer, unsigned int id, tdm_value value);
    tdm_error    (*layer_get_property)(tdm_layer *layer, unsigned int id, tdm_value *value);
    tdm_error    (*layer_set_info)(tdm_layer *layer, tdm_info_layer *info);
    tdm_error    (*layer_get_info)(tdm_layer *layer, tdm_info_layer *info);
    tdm_error    (*layer_set_buffer)(tdm_layer *layer, tdm_buffer *buffer);
    tdm_error    (*layer_unset_buffer)(tdm_layer *layer);
    tdm_error    (*layer_set_video_pos)(tdm_layer *layer, int zpos);
    tdm_capture *(*layer_create_capture)(tdm_layer *layer, tdm_error *error);
} tdm_func_display;

typedef void (*tdm_pp_done_handler)(tdm_pp *pp, tbm_surface_h src, tbm_surface_h dst, void *user_data);

typedef struct _tdm_func_pp
{
    void         (*pp_destroy)(tdm_pp *pp); /* init */
    tdm_error    (*pp_set_info)(tdm_pp *pp, tdm_info_pp *info);
    tdm_error    (*pp_attach)(tdm_pp *pp, tbm_surface_h src, tbm_surface_h dst);
    tdm_error    (*pp_commit)(tdm_pp *pp); /* init */
    tdm_error    (*pp_set_done_handler)(tdm_pp *pp, tdm_pp_done_handler func, void *user_data);
} tdm_func_pp;

typedef void (*tdm_capture_done_handler)(tdm_capture *capture, tdm_buffer *buffer, void *user_data);

typedef struct _tdm_func_capture
{
    void         (*capture_destroy)(tdm_capture *capture); /* init */
    tdm_error    (*capture_set_info)(tdm_capture *capture, tdm_info_capture *info);
    tdm_error    (*capture_attach)(tdm_capture *capture, tdm_buffer *buffer);
    tdm_error    (*capture_commit)(tdm_capture *capture); /* init */
    tdm_error    (*capture_set_done_handler)(tdm_capture *capture, tdm_capture_done_handler func, void *user_data);
} tdm_func_capture;

#define TDM_BACKEND_MINOR_VERSION_MASK  0x0000FFFF
#define TDM_BACKEND_MAJOR_VERSION_MASK  0xFFFF0000
#define TDM_BACKEND_GET_ABI_MINOR(v)    ((v) & TDM_BACKEND_MINOR_VERSION_MASK)
#define TDM_BACKEND_GET_ABI_MAJOR(v)    (((v) & TDM_BACKEND_MAJOR_VERSION_MASK) >> 16)

#define SET_TDM_BACKEND_ABI_VERSION(major, minor) \
        (((major) << 16) & TDM_BACKEND_MAJOR_VERSION_MASK) | \
        ((major) & TDM_BACKEND_MINOR_VERSION_MASK)
#define TDM_BACKEND_ABI_VERSION     SET_TDM_BACKEND_ABI_VERSION(1, 1)

typedef struct _tdm_backend_module
{
    const char *name;
    const char *vendor;
    unsigned long abi_version;

    tdm_backend_data* (*init)(tdm_display *dpy, tdm_error *error);
    void (*deinit)(tdm_backend_data *bdata);
} tdm_backend_module;

tdm_error tdm_backend_register_func_display(tdm_display *dpy, tdm_func_display *func_display);
tdm_error tdm_backend_register_func_pp(tdm_display *dpy, tdm_func_pp *func_pp);
tdm_error tdm_backend_register_func_capture(tdm_display *dpy, tdm_func_capture *func_capture);

typedef void (*tdm_buffer_destroy_handler)(tdm_buffer *buffer, void *user_data);

tdm_buffer*   tdm_buffer_ref_backend(tdm_buffer *buffer);
void          tdm_buffer_unref_backend(tdm_buffer *buffer);
tbm_surface_h tdm_buffer_get_surface(tdm_buffer *buffer);
tdm_buffer*   tdm_buffer_get(tbm_surface_h buffer);
tdm_error     tdm_buffer_add_destroy_handler(tdm_buffer *buffer, tdm_buffer_destroy_handler func, void *user_data);
void          tdm_buffer_remove_destroy_handler(tdm_buffer *buffer, tdm_buffer_destroy_handler func, void *user_data);


#ifdef __cplusplus
}
#endif

#endif /* _TDM_BACKEND_H_ */
