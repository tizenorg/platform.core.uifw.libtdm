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

#ifndef _TDM_H_
#define _TDM_H_

#include <stdint.h>
#include <tbm_surface.h>
#include <tbm_surface_internal.h>

#include "tdm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////

typedef enum
{
    TDM_DISPLAY_CAPABILITY_PP       = (1<<0),
    TDM_DISPLAY_CAPABILITY_CAPTURE  = (1<<1),
} tdm_display_capability;

/* not neccessary for user to know what backend is */
tdm_display* tdm_display_init(tdm_error *error);
void         tdm_display_deinit(tdm_display *dpy);
tdm_error    tdm_display_update(tdm_display *dpy);

///////////////////////////////////////////////////////////////////////////////

tdm_error    tdm_display_get_fd(tdm_display *dpy, int *fd);
tdm_error    tdm_display_handle_events(tdm_display *dpy);

tdm_error    tdm_display_get_capabilities(tdm_display *dpy, tdm_display_capability *capabilities);
tdm_error    tdm_display_get_pp_capabilities(tdm_display *dpy, tdm_pp_capability *capabilities);
tdm_error    tdm_display_get_pp_available_formats(tdm_display *dpy, const tbm_format **formats, int *count);
tdm_error    tdm_display_get_pp_available_size(tdm_display *dpy, int *min_w, int *min_h, int *max_w, int *max_h, int *preferred_align);
tdm_error    tdm_display_get_pp_available_properties(tdm_display *dpy, const tdm_prop **props, int *count);
tdm_error    tdm_display_get_capture_capabilities(tdm_display *dpy, tdm_capture_capability *capabilities);
tdm_error    tdm_display_get_catpure_available_formats(tdm_display *dpy, const tbm_format **formats, int *count);
tdm_error    tdm_display_get_output_count(tdm_display *dpy, int *count);
const tdm_output* tdm_display_get_output(tdm_display *dpy, int index, tdm_error *error);

tdm_pp*      tdm_display_create_pp(tdm_display *dpy, tdm_error *error);

///////////////////////////////////////////////////////////////////////////////

tdm_error   tdm_output_get_conn_status(tdm_output *output, tdm_output_conn_status *status);
tdm_error   tdm_output_get_output_type(tdm_output *output, tdm_output_type *type);
tdm_error   tdm_output_get_layer_count(tdm_output *output, int *count);
const tdm_layer* tdm_output_get_layer(tdm_output *output, int index, tdm_error *error);
tdm_error   tdm_output_get_available_properties(tdm_output *output, const tdm_prop **props, int *count);
tdm_error   tdm_output_get_available_modes(tdm_output *output, const tdm_output_mode **modes, int *count);
tdm_error   tdm_output_get_available_size(tdm_output *output, int *min_w, int *min_h, int *max_w, int *max_h, int *preferred_align);
tdm_error   tdm_output_get_physical_size(tdm_output *output, unsigned int *mmWidth, unsigned int *mmHeight);
tdm_error   tdm_output_get_subpixel(tdm_output *output, unsigned int *subpixel);
tdm_error   tdm_output_get_pipe(tdm_output *output, unsigned int *pipe);

tdm_error   tdm_output_set_property(tdm_output *output, unsigned int id, tdm_value value);
tdm_error   tdm_output_get_property(tdm_output *output, unsigned int id, tdm_value *value);
tdm_error   tdm_output_wait_vblank(tdm_output *output, int interval, int sync, tdm_output_vblank_handler func, void *user_data);
tdm_error   tdm_output_commit(tdm_output *output, int sync, tdm_output_commit_handler func, void *user_data);

tdm_error   tdm_output_set_mode(tdm_output *output, tdm_output_mode *mode);
tdm_error   tdm_output_get_mode(tdm_output *output, const tdm_output_mode **mode);

tdm_error   tdm_output_set_dpms(tdm_output *output, tdm_output_dpms dpms_value);
tdm_error   tdm_output_get_dpms(tdm_output *output, tdm_output_dpms *dpms_value);

tdm_capture *tdm_output_create_capture(tdm_output *output, tdm_error *error);

///////////////////////////////////////////////////////////////////////////////

tdm_error   tdm_layer_get_capabilities(tdm_layer *layer, tdm_layer_capability *capabilities);
tdm_error   tdm_layer_get_available_formats(tdm_layer *layer, const tbm_format **formats, int *count);
tdm_error   tdm_layer_get_available_properties(tdm_layer *layer, const tdm_prop **props, int *count);
tdm_error   tdm_layer_get_zpos(tdm_layer *layer, unsigned int *zpos); /* zpos of layer is fixed */

tdm_error   tdm_layer_set_property(tdm_layer *layer, unsigned int id, tdm_value value);
tdm_error   tdm_layer_get_property(tdm_layer *layer, unsigned int id, tdm_value *value);
tdm_error   tdm_layer_set_info(tdm_layer *layer, tdm_info_layer *info);
tdm_error   tdm_layer_get_info(tdm_layer *layer, tdm_info_layer *info);
tdm_error   tdm_layer_set_buffer(tdm_layer *layer, tdm_buffer *buffer); // layer has only one buffer
tdm_error   tdm_layer_unset_buffer(tdm_layer *layer);
tdm_error   tdm_layer_is_usable(tdm_layer *layer, unsigned int *usable);

tdm_capture *tdm_layer_create_capture(tdm_layer *layer, tdm_error *error);

///////////////////////////////////////////////////////////////////////////////

void         tdm_pp_destroy(tdm_pp *pp);
tdm_error    tdm_pp_set_property(tdm_pp *pp, unsigned int id, tdm_value value);
tdm_error    tdm_pp_get_property(tdm_pp *pp, unsigned int id, tdm_value *value);
tdm_error    tdm_pp_set_info(tdm_pp *pp, tdm_info_pp *info);
tdm_error    tdm_pp_attach(tdm_pp *pp, tdm_buffer *src, tdm_buffer *dst);
tdm_error    tdm_pp_commit(tdm_pp *pp);

///////////////////////////////////////////////////////////////////////////////

void         tdm_capture_destroy(tdm_capture *capture);
tdm_error    tdm_capture_set_info(tdm_capture *capture, tdm_info_capture *info);
tdm_error    tdm_capture_attach(tdm_capture *capture, tdm_buffer *buffer);
tdm_error    tdm_capture_commit(tdm_capture *capture);

///////////////////////////////////////////////////////////////////////////////

typedef void (*tdm_buffer_release_handler)(tdm_buffer *buffer, void *user_data);

tdm_buffer *tdm_buffer_create(tbm_surface_h buffer, tdm_error *error);
tdm_buffer *tdm_buffer_ref(tdm_buffer *buffer, tdm_error *error);
void        tdm_buffer_unref(tdm_buffer *buffer);
tdm_error   tdm_buffer_add_release_handler(tdm_buffer *buffer, tdm_buffer_release_handler func, void *user_data);
void        tdm_buffer_remove_release_handler(tdm_buffer *buffer, tdm_buffer_release_handler func, void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* _TDM_H_ */
