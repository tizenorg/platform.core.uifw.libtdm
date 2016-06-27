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

#ifndef _TDM_H_
#define _TDM_H_

#include <stdint.h>
#include <tbm_surface.h>
#include <tbm_surface_queue.h>
#include <tbm_surface_internal.h>

#include "tdm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file tdm.h
 * @brief The header file for a frontend user.
 * @par Example
 * @code
 * #include <tdm.h>    //for a frontend user
 * @endcode
 */

/**
 * @brief The display capability enumeration
 */
typedef enum {
	TDM_DISPLAY_CAPABILITY_PP       = (1 << 0), /**< if hardware supports pp operation */
	TDM_DISPLAY_CAPABILITY_CAPTURE  = (1 << 1), /**< if hardware supports capture operation */
} tdm_display_capability;

/**
 * @brief The output change handler
 * @details This handler will be called when the status of a output object is
 * changed in runtime.
 */
typedef void (*tdm_output_change_handler)(tdm_output *output,
										  tdm_output_change_type type,
										  tdm_value value,
										  void *user_data);

/**
 * @brief Initialize a display object
 * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
 * @return A display object
 * @see tdm_display_deinit
 */
tdm_display *
tdm_display_init(tdm_error *error);

/**
 * @brief Deinitialize a display object
 * @param[in] dpy A display object
 * @see tdm_display_init
 */
void
tdm_display_deinit(tdm_display *dpy);

/**
 * @brief Update a display object
 * @details
 * When new output is connected, a frontend user need to call this function.
 * And a frontend user can the new output information with tdm_output_get_xxx functions.
 * @param[in] dpy A display object
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_display_update(tdm_display *dpy);

/**
 * @brief Get the file descriptor
 * @details TDM handles the events of fd with #tdm_display_handle_events.
 * @param[in] dpy A display object
 * @param[out] fd The file descriptor
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_display_handle_events
 */
tdm_error
tdm_display_get_fd(tdm_display *dpy, int *fd);

/**
 * @brief Handle the events
 * @param[in] dpy A display object
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_display_get_fd
 */
tdm_error
tdm_display_handle_events(tdm_display *dpy);

/**
 * @brief Get the capabilities of a display object.
 * @details A frontend user can get whether TDM supports pp/capture functionality with this function.
 * @param[in] dpy A display object
 * @param[out] capabilities The capabilities of a display object
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_display_get_capabilities(tdm_display *dpy,
							 tdm_display_capability *capabilities);

/**
 * @brief Get the pp capabilities of a display object.
 * @param[in] dpy A display object
 * @param[out] capabilities The pp capabilities
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_display_get_pp_capabilities(tdm_display *dpy,
								tdm_pp_capability *capabilities);

/**
 * @brief Get the pp available format array of a display object.
 * @param[in] dpy A display object
 * @param[out] formats The pp available format array
 * @param[out] count The count of formats
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_display_get_pp_available_formats(tdm_display *dpy,
									 const tbm_format **formats, int *count);

/**
 * @brief Get the pp available size of a display object.
 * @details -1 means that a TDM backend module doesn't define the value.
 * @param[in] dpy A display object
 * @param[out] min_w The minimum width which TDM can handle
 * @param[out] min_h The minimum height which TDM can handle
 * @param[out] max_w The maximum width which TDM can handle
 * @param[out] max_h The maximum height which TDM can handle
 * @param[out] preferred_align The preferred align width which TDM can handle
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_display_get_pp_available_size(tdm_display *dpy, int *min_w, int *min_h,
								  int *max_w, int *max_h, int *preferred_align);

/**
 * @brief Get the capture capabilities of a display object.
 * @param[in] dpy A display object
 * @param[out] capabilities The capture capabilities
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_display_get_capture_capabilities(tdm_display *dpy,
									 tdm_capture_capability *capabilities);

/**
 * @brief Get the capture available format array of a display object.
 * @param[in] dpy A display object
 * @param[out] formats The capture available format array
 * @param[out] count The count of formats
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_display_get_catpure_available_formats(tdm_display *dpy,
										  const tbm_format **formats, int *count);

/**
 * @brief Get the output counts which a display object has.
 * @param[in] dpy A display object
 * @param[out] count The count of outputs
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_display_get_output
 */
tdm_error
tdm_display_get_output_count(tdm_display *dpy, int *count);

/**
 * @brief Get a output object which has the given index.
 * @param[in] dpy A display object
 * @param[in] index The index of a output object
 * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
 * @return A output object if success. Otherwise, NULL.
 * @see tdm_display_get_output_count
 */
tdm_output *
tdm_display_get_output(tdm_display *dpy, int index, tdm_error *error);

/**
 * @brief Create a pp object.
 * @param[in] dpy A display object
 * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
 * @return A pp object if success. Otherwise, NULL.
 * @see tdm_pp_destroy
 */
tdm_pp *
tdm_display_create_pp(tdm_display *dpy, tdm_error *error);

/**
 * @brief Get the model information of a output object.
 * @param[in] output A output object
 * @param[out] maker The output maker.
 * @param[out] model The output model.
 * @param[out] name The output name.
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_output_get_model_info(tdm_output *output, const char **maker,
						  const char **model, const char **name);

/**
 * @brief Get the connection status of a output object.
 * @param[in] output A output object
 * @param[out] status The connection status.
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_output_get_conn_status(tdm_output *output, tdm_output_conn_status *status);

/**
 * @brief Add a output change handler
 * @details The handler will be called when the status of a
 * output object is changed. connection, DPMS, etc.
 * @param[in] output A output object
 * @param[in] func A output change handler
 * @param[in] user_data The user data
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_output_add_change_handler(tdm_output *output,
							  tdm_output_change_handler func,
							  void *user_data);

/**
 * @brief Remove a output change handler
 * @param[in] output A output object
 * @param[in] func A output change handler
 * @param[in] user_data The user data
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
void
tdm_output_remove_change_handler(tdm_output *output,
								 tdm_output_change_handler func,
								 void *user_data);

/**
 * @brief Get the connection type of a output object.
 * @param[in] output A output object
 * @param[out] type The connection type.
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_output_get_output_type(tdm_output *output, tdm_output_type *type);

/**
 * @brief Get the layer counts which a output object has.
 * @param[in] output A output object
 * @param[out] count The count of layers
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_output_get_layer
 */
tdm_error
tdm_output_get_layer_count(tdm_output *output, int *count);

/**
 * @brief Get a layer object which has the given index.
 * @param[in] output A output object
 * @param[in] index The index of a layer object
 * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
 * @return A layer object if success. Otherwise, NULL.
 * @see tdm_output_get_layer_count
 */
tdm_layer *
tdm_output_get_layer(tdm_output *output, int index, tdm_error *error);

/**
 * @brief Get the available property array of a output object.
 * @param[in] output A output object
 * @param[out] props The available property array
 * @param[out] count The count of properties
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_output_get_available_properties(tdm_output *output, const tdm_prop **props,
									int *count);

/**
 * @brief Get the available mode array of a output object.
 * @param[in] output A output object
 * @param[out] modes The available mode array
 * @param[out] count The count of modes
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_output_get_available_modes(tdm_output *output,
							   const tdm_output_mode **modes, int *count);

/**
 * @brief Get the available size of a output object.
 * @details -1 means that a TDM backend module doesn't define the value.
 * @param[in] output A output object
 * @param[out] min_w The minimum width which TDM can handle
 * @param[out] min_h The minimum height which TDM can handle
 * @param[out] max_w The maximum width which TDM can handle
 * @param[out] max_h The maximum height which TDM can handle
 * @param[out] preferred_align The preferred align width which TDM can handle
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_output_get_available_size(tdm_output *output, int *min_w, int *min_h,
							  int *max_w, int *max_h, int *preferred_align);

/**
 * @brief Get the physical size of a output object.
 * @param[in] output A output object
 * @param[out] mmWidth The milimeter width
 * @param[out] mmHeight The milimeter height
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_output_get_physical_size(tdm_output *output, unsigned int *mmWidth,
							 unsigned int *mmHeight);

/**
 * @brief Get the subpixel of a output object.
 * @param[in] output A output object
 * @param[out] subpixel The subpixel
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_output_get_subpixel(tdm_output *output, unsigned int *subpixel);

/**
 * @brief Get the pipe of a output object.
 * @param[in] output A output object
 * @param[out] pipe The pipe
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_output_get_pipe(tdm_output *output, unsigned int *pipe);

/**
 * @brief Set the property which has a given id.
 * @param[in] output A output object
 * @param[in] id The property id
 * @param[in] value The value
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_output_set_property(tdm_output *output, unsigned int id, tdm_value value);

/**
 * @brief Get the property which has a given id
 * @param[in] output A output object
 * @param[in] id The property id
 * @param[out] value The value
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_output_get_property(tdm_output *output, unsigned int id, tdm_value *value);

/**
 * @brief Wait for VBLANK
 * @details After interval vblanks, a user vblank handler will be called.
 * @param[in] output A output object
 * @param[in] interval vblank interval
 * @param[in] sync 0: asynchronous, 1:synchronous
 * @param[in] func A user vblank handler
 * @param[in] user_data The user data
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see #tdm_output_vblank_handler
 */
tdm_error
tdm_output_wait_vblank(tdm_output *output, int interval, int sync,
					   tdm_output_vblank_handler func, void *user_data);

/**
 * @brief Commit changes for a output object
 * @details After all change of a output object are applied, a user commit handler
 * will be called.
 * @param[in] output A output object
 * @param[in] sync 0: asynchronous, 1:synchronous
 * @param[in] func A user commit handler
 * @param[in] user_data The user data
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_output_commit(tdm_output *output, int sync, tdm_output_commit_handler func,
				  void *user_data);

/**
 * @brief Set one of available modes of a output object
 * @param[in] output A output object
 * @param[in] mode A output mode
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_output_set_mode(tdm_output *output, const tdm_output_mode *mode);

/**
 * @brief Get the mode of a output object
 * @param[in] output A output object
 * @param[out] mode A output mode
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_output_get_mode(tdm_output *output, const tdm_output_mode **mode);

/**
 * @brief Set DPMS of a output object
 * @param[in] output A output object
 * @param[in] dpms_value DPMS value
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_output_set_dpms(tdm_output *output, tdm_output_dpms dpms_value);

/**
 * @brief Get DPMS of a output object
 * @param[in] output A output object
 * @param[out] dpms_value DPMS value
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_output_get_dpms(tdm_output *output, tdm_output_dpms *dpms_value);

/**
 * @brief Create a capture object of a output object
 * @param[in] output A output object
 * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
 * @return A capture object
 * @see tdm_capture_destroy
 */
tdm_capture *
tdm_output_create_capture(tdm_output *output, tdm_error *error);

/**
 * @brief Get the capabilities of a layer object.
 * @param[in] layer A layer object
 * @param[out] capabilities The capabilities of a layer object
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_layer_get_capabilities(tdm_layer *layer,
						   tdm_layer_capability *capabilities);

/**
 * @brief Get the available format array of a layer object.
 * @param[in] layer A layer object
 * @param[out] formats The available format array
 * @param[out] count The count of formats
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_layer_get_available_formats(tdm_layer *layer, const tbm_format **formats,
								int *count);

/**
 * @brief Get the available property array of a layer object.
 * @param[in] layer A layer object
 * @param[out] props The available property array
 * @param[out] count The count of properties
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_layer_get_available_properties(tdm_layer *layer, const tdm_prop **props,
								   int *count);

/**
 * @brief Get the zpos of a layer object.
 * @details
 * - GRAPHIC layers have fixed zpos. It starts from 0. It's @b non-changeable.
 * - But the zpos of VIDEO layers will be decided by a backend module side.
 * - A frontend user only can set the relative zpos to VIDEO layers via #tdm_layer_set_video_pos
 * - The zpos of video layers is less than GRAPHIC layers or more than GRAPHIC
 * layers. ie, ..., -2, -1, 4, 5, ... (if 0 <= GRAPHIC layer's zpos < 4).
 *   -------------------------------- graphic layer  3 <-- top most layer
 *   -------------------------------- graphic layer  2
 *   -------------------------------- graphic layer  1
 *   -------------------------------- graphic layer  0
 *   -------------------------------- video   layer -1
 *   -------------------------------- video   layer -2 <-- lowest layer
 * @param[in] layer A layer object
 * @param[out] zpos The zpos
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_layer_set_video_pos, tdm_layer_capability
 */
tdm_error
tdm_layer_get_zpos(tdm_layer *layer, int *zpos);

/**
 * @brief Set the property which has a given id.
 * @param[in] layer A layer object
 * @param[in] id The property id
 * @param[in] value The value
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_layer_set_property(tdm_layer *layer, unsigned int id, tdm_value value);

/**
 * @brief Get the property which has a given id.
 * @param[in] layer A layer object
 * @param[in] id The property id
 * @param[out] value The value
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_layer_get_property(tdm_layer *layer, unsigned int id, tdm_value *value);

/**
 * @brief Set the geometry information to a layer object
 * @details The geometry information will be applied when the output object
 * of a layer object is committed. If a layer has TDM_LAYER_CAPABILITY_NO_CROP
 * capability, a layer will ignore the pos(crop) information of #tdm_info_layer's
 * src_config.
 * @param[in] layer A layer object
 * @param[in] info The geometry information
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_output_commit
 */
tdm_error
tdm_layer_set_info(tdm_layer *layer, tdm_info_layer *info);

/**
 * @brief Get the geometry information to a layer object
 * @param[in] layer A layer object
 * @param[out] info The geometry information
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_layer_get_info(tdm_layer *layer, tdm_info_layer *info);

/**
 * @brief Set a TDM buffer to a layer object
 * @details A TDM buffer will be applied when the output object
 * of a layer object is committed.
 * @param[in] layer A layer object
 * @param[in] buffer A TDM buffer
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_output_commit
 */
tdm_error
tdm_layer_set_buffer(tdm_layer *layer, tbm_surface_h buffer);

/**
 * @brief Unset a TDM buffer from a layer object
 * @details When this function is called, a current showing buffer will be
 * disappeared from screen. Then nothing is showing on a layer object.
 * @param[in] layer A layer object
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_layer_unset_buffer(tdm_layer *layer);

/**
 * @brief Get a displaying TDM buffer from a layer object
 * @details A displaying TDM buffer is a current showing buffer on screen
 * that is set to layer object and applied output object of a layer object.
 * @param[in] layer A layer object
 * @return A TDM buffer if success. Otherwise, NULL.
 */
tbm_surface_h
tdm_layer_get_displaying_buffer(tdm_layer *layer, tdm_error *error);

/**
 * @brief Set a TBM surface_queue to a layer object
 * @details A TBM surface_queue will be applied when the output object
 * of a layer object is committed. and TDM layer will be automatically updated
 * @param[in] layer A layer object
 * @param[in] buffer_queue A TBM surface_queue
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_output_commit
 */
tdm_error
tdm_layer_set_buffer_queue(tdm_layer *layer, tbm_surface_queue_h buffer_queue);

/**
 * @brief Unset a TBM surface_queue from a layer object
 * @details When this function is called, a current surface_queue will be
 * disappeared from screen. Then nothing is showing on a layer object.
 * @param[in] layer A layer object
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_layer_unset_buffer_queue(tdm_layer *layer);

/**
 * @brief Check wheter a layer object is available for a frontend user to use.
 * @details A layer object is not usable if a TDM buffer is showing on screen
 * via this layer object. By calling #tdm_layer_unset_buffer, this layer object
 * will become usable.
 * @param[in] layer A layer object
 * @param[out] usable 1 if usable, 0 if not usable
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_layer_is_usable(tdm_layer *layer, unsigned int *usable);

/**
 * @brief Set the relative zpos to a VIDEO layer object
 * @details The zpos value is less than the minimum zpos of GRAPHIC layers, or
 * it is more than the maximum zpos of GRAPHIC layers.
 * @param[in] layer A VIDEO layer object
 * @param[in] zpos The zpos
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_layer_get_zpos, tdm_layer_capability
 */
tdm_error
tdm_layer_set_video_pos(tdm_layer *layer, int zpos);

/**
 * @brief Create a capture object of a layer object
 * @param[in] layer A layer object
 * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
 * @return A capture object
 * @see tdm_capture_destroy
 */
tdm_capture *
tdm_layer_create_capture(tdm_layer *layer, tdm_error *error);

/**
 * @brief Get buffer flags from a layer object
 * @param[in] layer A layer object
 * @param[out] flags a buffer flags value
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_layer_get_buffer_flags(tdm_layer *layer, unsigned int *flags);

/**
 * @brief Destroy a pp object
 * @param[in] pp A pp object
 * @see tdm_display_create_pp
 */
void
tdm_pp_destroy(tdm_pp *pp);

/**
 * @brief Set the geometry information to a pp object
 * @param[in] pp A pp object
 * @param[in] info The geometry information
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_pp_commit
 */
tdm_error
tdm_pp_set_info(tdm_pp *pp, tdm_info_pp *info);

/**
 * @brief Attach a source buffer and a destination buffer to a pp object
 * @param[in] pp A pp object
 * @param[in] src A source buffer
 * @param[in] dst A destination buffer
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_pp_commit, tdm_buffer_add_release_handler, tdm_buffer_release_handler
 */
tdm_error
tdm_pp_attach(tdm_pp *pp, tbm_surface_h src, tbm_surface_h dst);

/**
 * @brief Commit changes for a pp object
 * @param[in] pp A pp object
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_pp_commit(tdm_pp *pp);

/**
 * @brief Destroy a capture object
 * @param[in] capture A capture object
 * @see tdm_output_create_capture, tdm_layer_create_capture
 */
void
tdm_capture_destroy(tdm_capture *capture);

/**
 * @brief Set the geometry information to a capture object
 * @param[in] capture A capture object
 * @param[in] info The geometry information
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_capture_commit
 */
tdm_error
tdm_capture_set_info(tdm_capture *capture, tdm_info_capture *info);

/**
 * @brief Attach a TDM buffer to a capture object
 * @param[in] capture A capture object
 * @param[in] buffer A TDM buffer
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_capture_commit, tdm_buffer_add_release_handler, tdm_buffer_release_handler
 */
tdm_error
tdm_capture_attach(tdm_capture *capture, tbm_surface_h buffer);

/**
 * @brief Commit changes for a capture object
 * @param[in] capture A capture object
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_capture_commit(tdm_capture *capture);

/**
 * @brief The release handler of a TDM buffer
 * @param[in] buffer A TDM buffer
 * @param[in] user_data user data
 * @see tdm_buffer_add_release_handler, tdm_buffer_remove_release_handler
 */
typedef void (*tdm_buffer_release_handler)(tbm_surface_h buffer,
										   void *user_data);

/**
 * @brief Add a release handler to a TDM buffer
 * @details
 * TDM has its own buffer release mechanism to let an frontend user know when a TDM buffer
 * becomes available for a next job. A TDM buffer can be used for TDM to show
 * it on screen or to capture an output and a layer. After all operations,
 * TDM will release it immediately when TDM doesn't need it any more.
 * @param[in] buffer A TDM buffer
 * @param[in] func A release handler
 * @param[in] user_data user data
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_buffer_remove_release_handler
 */
tdm_error
tdm_buffer_add_release_handler(tbm_surface_h buffer,
							   tdm_buffer_release_handler func, void *user_data);

/**
 * @brief Remove a release handler from a TDM buffer
 * @param[in] buffer A TDM buffer
 * @param[in] func A release handler
 * @param[in] user_data user data
 * @see tdm_buffer_add_release_handler
 */
void
tdm_buffer_remove_release_handler(tbm_surface_h buffer,
								  tdm_buffer_release_handler func, void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* _TDM_H_ */
