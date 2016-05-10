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

/**
 * @file tdm_backend.h
 * @brief The backend header file of TDM to implement a TDM backend module.
 * @par Example
 * @code
   #include <tdm_backend.h>
 * @endcode
 */

/**
 * @brief The backend module data
 * @details
 * The init() function of #tdm_backend_module returns the backend module data.
 * And it will be passed to display functions of #tdm_func_display.
 * @see tdm_backend_module, tdm_backend_module
 */
typedef void tdm_backend_data;

/**
 * @brief The output status handler
 * @details This handler will be called when the status of a output object is
 * changed in runtime.
 */
typedef void (*tdm_output_status_handler)(tdm_output *output,
                                          tdm_output_conn_status status,
                                          void *user_data);

/**
 * @brief The display capabilities structure of a backend module
 * @see The display_get_capabilitiy() function of #tdm_func_display
 */
typedef struct _tdm_caps_display {
	int max_layer_count;    /**< The maximum layer count. -1 means "not defined" */
} tdm_caps_display;

/**
 * @brief The capabilities structure of a output object
 * @see The output_get_capability() function of #tdm_func_output
 */
typedef struct _tdm_caps_output {
	char maker[TDM_NAME_LEN];       /**< The output maker */
	char model[TDM_NAME_LEN];       /**< The output model */
	char name[TDM_NAME_LEN];        /**< The output name */

	tdm_output_conn_status status;  /**< The connection status */
	tdm_output_type type;           /**< The connection type */
	unsigned int type_id;           /**< The connection type id */

	unsigned int mode_count;        /**< The count of available modes */
	tdm_output_mode
	*modes;         /**< The @b newly-allocated array of modes. will be freed in frontend. */

	unsigned int prop_count;        /**< The count of available properties */
	tdm_prop *props;                /**< The @b newly-allocated array of properties. will be freed in frontend. */

	unsigned int mmWidth;           /**< The physical width (milimeter) */
	unsigned int mmHeight;          /**< The physical height (milimeter) */
	unsigned int subpixel;          /**< The subpixel */

	int min_w;              /**< The minimun width. -1 means "not defined" */
	int min_h;              /**< The minimun height. -1 means "not defined" */
	int max_w;              /**< The maximum width. -1 means "not defined" */
	int max_h;              /**< The maximum height. -1 means "not defined" */
	int preferred_align;    /**< The prefered align. -1 means "not defined" */
} tdm_caps_output;

/**
 * @brief The capabilities structure of a layer object
 * @see The layer_get_capability() function of #tdm_func_layer
 */
typedef struct _tdm_caps_layer {
	tdm_layer_capability capabilities;  /**< The capabilities of layer */

	/**
	 * The z-order
	 * GRAPHIC layers are non-changeable. The zpos of GRAPHIC layers starts
	 * from 0. If there are 4 GRAPHIC layers, The zpos SHOULD be 0, 1, 2, 3.\n
	 * But the zpos of VIDEO layer is changeable by layer_set_video_pos() function
	 * of #tdm_func_layer. And The zpos of VIDEO layers is less than GRAPHIC
	 * layers or more than GRAPHIC layers.
	 * ie, ..., -2, -1, 4, 5, ... (if 0 <= GRAPHIC layer's zpos < 4).
	 * The zpos of VIDEO layers is @b relative. It doesn't need to start
	 * from -1 or 4. Let's suppose that there are two VIDEO layers.
	 * One has -2 zpos. Another has -4 zpos. Then -2 Video layer is higher
	 * than -4 VIDEO layer.
	*/
	int zpos;

	unsigned int format_count;      /**< The count of available formats */
	tbm_format
	*formats;            /**< The @b newly-allocated array of formats. will be freed in frontend. */

	unsigned int prop_count;        /**< The count of available properties */
	tdm_prop *props;                /**< The @b newly-allocated array of properties. will be freed in frontend. */
} tdm_caps_layer;

/**
 * @brief The capabilities structure of a pp object
 * @see The display_get_pp_capability() function of #tdm_func_display
 */
typedef struct _tdm_caps_pp {
	tdm_pp_capability capabilities; /**< The capabilities of pp */

	unsigned int format_count;      /**< The count of available formats */
	tbm_format
	*formats;            /**< The @b newly-allocated array. will be freed in frontend. */

	int min_w;              /**< The minimun width. -1 means "not defined" */
	int min_h;              /**< The minimun height. -1 means "not defined" */
	int max_w;              /**< The maximum width. -1 means "not defined" */
	int max_h;              /**< The maximum height. -1 means "not defined" */
	int preferred_align;    /**< The prefered align. -1 means "not defined" */
} tdm_caps_pp;

/**
 * @brief The capabilities structure of a capture object
 * @see The display_get_capture_capability() function of #tdm_func_display
 */
typedef struct _tdm_caps_capture {
	tdm_capture_capability capabilities;    /**< The capabilities of capture */

	unsigned int format_count;      /**< The count of available formats */
	tbm_format
	*formats;            /**< The @b newly-allocated array of formats. will be freed in frontend. */

	int min_w;              /**< The minimun width. -1 means "not defined" */
	int min_h;              /**< The minimun height. -1 means "not defined" */
	int max_w;              /**< The maximum width. -1 means "not defined" */
	int max_h;              /**< The maximum height. -1 means "not defined" */
	int preferred_align;    /**< The prefered align. -1 means "not defined" */
} tdm_caps_capture;

/**
 * @brief The display functions for a backend module.
 */
typedef struct _tdm_func_display {
	/**
	 * @brief Get the display capabilities of a backend module
	 * @param[in] bdata The backend module data
	 * @param[out] caps The display capabilities of a backend module
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @remark
	 * A backend module @b SHOULD implement this function. TDM calls this function
	 * not only at the initial time, but also at the update time when new output
	 * is connected.\n
	 * If a hardware has the restriction of the number of max usable layer count,
	 * a backend module can set the max count to max_layer_count of #tdm_caps_display
	 * structure. Otherwise, set -1.
	 */
	tdm_error (*display_get_capabilitiy)(tdm_backend_data *bdata,
	                                     tdm_caps_display *caps);

	/**
	 * @brief Get the pp capabilities of a backend module
	 * @param[in] bdata The backend module data
	 * @param[out] caps The pp capabilities of a backend module
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @see tdm_backend_register_func_pp
	 * @remark
	 * TDM calls this function not only at the initial time, but also at the update
	 * time when new output is connected.\n
	 * A backend module doesn't need to implement this function if a hardware
	 * doesn't have the memory-to-memory converting device. Otherwise, a backend module
	 * @b SHOULD fill the #tdm_caps_pp data. #tdm_caps_pp contains the hardware
	 * restriction information which a converting device can handle. ie, format, size, etc.
	 */
	tdm_error (*display_get_pp_capability)(tdm_backend_data *bdata,
	                                       tdm_caps_pp *caps);

	/**
	 * @brief Get the capture capabilities of a backend module
	 * @param[in] bdata The backend module data
	 * @param[out] caps The capture capabilities of a backend module
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @see tdm_backend_register_func_capture
	 * @remark
	 * TDM calls this function not only at the initial time, but also at the update
	 * time when new output is connected.\n
	 * A backend module doesn't need to implement this function if a hardware
	 * doesn't have the capture device. Otherwise, a backend module @b SHOULD fill the
	 * #tdm_caps_capture data. #tdm_caps_capture contains the hardware restriction
	 * information which a capture device can handle. ie, format, size, etc.
	 */
	tdm_error (*display_get_capture_capability)(tdm_backend_data *bdata,
	                tdm_caps_capture *caps);

	/**
	 * @brief Get a output array of a backend module
	 * @param[in] bdata The backend module data
	 * @param[out] count The count of outputs
	 * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @return A output array which is @b newly-allocated
	 * @see tdm_backend_register_func_capture
	 * @remark
	 * A backend module @b SHOULD implement this function. TDM calls this function
	 * not only at the initial time, but also at the update time when new output
	 * is connected.\n
	 * A backend module @b SHOULD return the @b newly-allocated array which contains
	 * "tdm_output*" data. It will be freed in frontend.
	 * @par Example
	 * @code
	    tdm_output**
	    drm_display_get_outputs(tdm_backend_data *bdata, int *count, tdm_error *error)
	    {
	        tdm_drm_data *drm_data = bdata;
	        tdm_drm_output_data *output_data = NULL;
	        tdm_output **outputs;
	        int i;

	        (*count) = 0;
	        LIST_FOR_EACH_ENTRY(output_data, &drm_data->output_list, link)
	            (*count)++;

	        if ((*count) == 0)
	        {
	            if (error) *error = TDM_ERROR_NONE;
	            return NULL;
	        }

	        // will be freed in frontend
	        outputs = calloc(*count, sizeof(tdm_drm_output_data*));
	        if (!outputs)
	        {
	            (*count) = 0;
	            if (error) *error = TDM_ERROR_OUT_OF_MEMORY;
	            return NULL;
	        }

	        i = 0;
	        LIST_FOR_EACH_ENTRY(output_data, &drm_data->output_list, link)
	            outputs[i++] = output_data;

	        if (error) *error = TDM_ERROR_NONE;

	        return outputs;
	    }
	 * @endcode
	 */
	tdm_output **(*display_get_outputs)(tdm_backend_data *bdata, int *count,
	                                    tdm_error *error);

	/**
	 * @brief Get the file descriptor of a backend module
	 * @param[in] bdata The backend module data
	 * @param[out] fd The fd of a backend module
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @see display_handle_events() function of #tdm_func_display
	 * @remark
	 * A backend module can return epoll's fd which is watching the backend device one more fds.
	 */
	tdm_error (*display_get_fd)(tdm_backend_data *bdata, int *fd);

	/**
	 * @brief Handle the events which happens on the fd of a backend module
	 * @param[in] bdata The backend module data
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 */
	tdm_error (*display_handle_events)(tdm_backend_data *bdata);

	/**
	 * @brief Create a pp object of a backend module
	 * @param[in] bdata The backend module data
	 * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @return A pp object
	 * @see pp_destroy() function of #tdm_func_pp
	 * @remark
	 * A backend module doesn't need to implement this function if a hardware
	 * doesn't have the memory-to-memory converting device.
	 */
	tdm_pp *(*display_create_pp)(tdm_backend_data *bdata, tdm_error *error);

	void (*reserved1)(void);
	void (*reserved2)(void);
	void (*reserved3)(void);
	void (*reserved4)(void);
	void (*reserved5)(void);
	void (*reserved6)(void);
	void (*reserved7)(void);
	void (*reserved8)(void);
} tdm_func_display;

/**
 * @brief The output functions for a backend module.
 */
typedef struct _tdm_func_output {
	/**
	 * @brief Get the capabilities of a output object
	 * @param[in] output A output object
	 * @param[out] caps The capabilities of a output object
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @remark
	 * A backend module @b SHOULD implement this function. TDM calls this function
	 * not only at the initial time, but also at the update time when new output
	 * is connected.\n
	 * #tdm_caps_output contains connection status, modes, avaiable properties,
	 * size restriction information, etc.
	 */
	tdm_error (*output_get_capability)(tdm_output *output, tdm_caps_output *caps);

	/**
	 * @brief Get a layer array of a output object
	 * @param[in] output A output object
	 * @param[out] count The count of layers
	 * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @return A layer array which is @b newly-allocated
	 * @remark
	 * A backend module @b SHOULD implement this function. TDM calls this function
	 * not only at the initial time, but also at the update time when new output
	 * is connected.\n
	 * A backend module @b SHOULD return the @b newly-allocated array which contains
	 * "tdm_layer*" data. It will be freed in frontend.
	 */
	tdm_layer **(*output_get_layers)(tdm_output *output, int *count,
	                                 tdm_error *error);

	/**
	 * @brief Set the property which has a given id
	 * @param[in] output A output object
	 * @param[in] id The property id
	 * @param[in] value The value
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 */
	tdm_error (*output_set_property)(tdm_output *output, unsigned int id,
	                                 tdm_value value);

	/**
	 * @brief Get the property which has a given id
	 * @param[in] output A output object
	 * @param[in] id The property id
	 * @param[out] value The value
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 */
	tdm_error (*output_get_property)(tdm_output *output, unsigned int id,
	                                 tdm_value *value);

	/**
	 * @brief Wait for VBLANK
	 * @param[in] output A output object
	 * @param[in] interval vblank interval
	 * @param[in] sync 0: asynchronous, 1:synchronous
	 * @param[in] user_data The user data
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @see output_set_vblank_handler, tdm_output_vblank_handler
	 * @remark
	 * A backend module @b SHOULD call a user vblank handler after interval vblanks.
	 */
	tdm_error (*output_wait_vblank)(tdm_output *output, int interval, int sync,
	                                void *user_data);

	/**
	 * @brief Set a user vblank handler
	 * @param[in] output A output object
	 * @param[in] func A user vblank handler
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 */
	tdm_error (*output_set_vblank_handler)(tdm_output *output,
	                                       tdm_output_vblank_handler func);

	/**
	 * @brief Commit changes for a output object
	 * @param[in] output A output object
	 * @param[in] sync 0: asynchronous, 1:synchronous
	 * @param[in] user_data The user data
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @see output_set_commit_handler, tdm_output_commit_handler
	 * @remark
	 * A backend module @b SHOULD call a user commit handler after all change of
	 * a output object are applied.
	 */
	tdm_error (*output_commit)(tdm_output *output, int sync, void *user_data);

	/**
	 * @brief Set a user commit handler
	 * @param[in] output A output object
	 * @param[in] func A user commit handler
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 */
	tdm_error (*output_set_commit_handler)(tdm_output *output,
	                                       tdm_output_commit_handler func);

	/**
	 * @brief Set DPMS of a output object
	 * @param[in] output A output object
	 * @param[in] dpms_value DPMS value
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 */
	tdm_error (*output_set_dpms)(tdm_output *output, tdm_output_dpms dpms_value);

	/**
	 * @brief Get DPMS of a output object
	 * @param[in] output A output object
	 * @param[out] dpms_value DPMS value
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 */
	tdm_error (*output_get_dpms)(tdm_output *output, tdm_output_dpms *dpms_value);

	/**
	 * @brief Set one of available modes of a output object
	 * @param[in] output A output object
	 * @param[in] mode A output mode
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 */
	tdm_error (*output_set_mode)(tdm_output *output, const tdm_output_mode *mode);

	/**
	 * @brief Get the mode of a output object
	 * @param[in] output A output object
	 * @param[out] mode A output mode
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 */
	tdm_error (*output_get_mode)(tdm_output *output, const tdm_output_mode **mode);

	/**
	 * @brief Create a capture object of a output object
	 * @param[in] output A output object
	 * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @return A capture object
	 * @see capture_destroy() function of #tdm_func_capture
	 * @remark
	 * A backend module doesn't need to implement this function if a hardware
	 * doesn't have the capture device.
	 */
	tdm_capture *(*output_create_capture)(tdm_output *output, tdm_error *error);

	/**
	 * @brief Set a output connection status handler
	 * @details A backend module need to call the output status handler when the
	 * output connection status has been changed to let the TDM frontend know
	 * the change.
	 * @param[in] output A output object
	 * @param[in] func A output status handler
	 * @param[in] user_data The user data
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 */
	tdm_error (*output_set_status_handler)(tdm_output *output,
	                                       tdm_output_status_handler func,
	                                       void *user_data);

	void (*reserved1)(void);
	void (*reserved2)(void);
	void (*reserved3)(void);
	void (*reserved4)(void);
	void (*reserved5)(void);
	void (*reserved6)(void);
	void (*reserved7)(void);
	void (*reserved8)(void);
} tdm_func_output;

/**
 * @brief The layer functions for a backend module.
 */
typedef struct _tdm_func_layer {
	/**
	 * @brief Get the capabilities of a layer object
	 * @param[in] layer A layer object
	 * @param[out] caps The capabilities of a layer object
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @remark
	 * A backend module @b SHOULD implement this function. TDM calls this function
	 * not only at the initial time, but also at the update time when new output
	 * is connected.\n
	 * #tdm_caps_layer contains avaiable formats/properties, zpos information, etc.
	 */
	tdm_error (*layer_get_capability)(tdm_layer *layer, tdm_caps_layer *caps);

	/**
	 * @brief Set the property which has a given id.
	 * @param[in] layer A layer object
	 * @param[in] id The property id
	 * @param[in] value The value
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 */
	tdm_error (*layer_set_property)(tdm_layer *layer, unsigned int id,
	                                tdm_value value);

	/**
	 * @brief Get the property which has a given id.
	 * @param[in] layer A layer object
	 * @param[in] id The property id
	 * @param[out] value The value
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 */
	tdm_error (*layer_get_property)(tdm_layer *layer, unsigned int id,
	                                tdm_value *value);

	/**
	 * @brief Set the geometry information to a layer object
	 * @param[in] layer A layer object
	 * @param[in] info The geometry information
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @see output_commit() function of #tdm_func_output
	 * @remark
	 * A backend module would apply the geometry information when the output object
	 * of a layer object is committed.
	 */
	tdm_error (*layer_set_info)(tdm_layer *layer, tdm_info_layer *info);

	/**
	 * @brief Get the geometry information to a layer object
	 * @param[in] layer A layer object
	 * @param[out] info The geometry information
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 */
	tdm_error (*layer_get_info)(tdm_layer *layer, tdm_info_layer *info);

	/**
	 * @brief Set a TDM buffer to a layer object
	 * @param[in] layer A layer object
	 * @param[in] buffer A TDM buffer
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @see output_commit() function of #tdm_func_output
	 * @remark
	 * A backend module would apply a TDM buffer when the output object
	 * of a layer object is committed.
	 */
	tdm_error (*layer_set_buffer)(tdm_layer *layer, tbm_surface_h buffer);

	/**
	 * @brief Unset a TDM buffer from a layer object
	 * @param[in] layer A layer object
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @remark
	 * A backend module @b SHOULD hide the current showing buffer from screen.
	 * If needed, cleanup a layer object resource.
	 */
	tdm_error (*layer_unset_buffer)(tdm_layer *layer);

	/**
	 * @brief Set the zpos for a VIDEO layer object
	 * @param[in] layer A layer object
	 * @param[in] zpos z-order
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @see tdm_caps_layer, tdm_layer_capability
	 * @remark
	 * A backend module doesn't need to implement this function if a backend
	 * module doesn't have VIDEO layers.\n
	 * This function is for VIDEO layers.
	 * GRAPHIC layers are non-changeable. The zpos of GRAPHIC layers starts
	 * from 0. If there are 4 GRAPHIC layers, The zpos SHOULD be 0, 1, 2, 3.\n
	 * But the zpos of VIDEO layer is changeable. And The zpos of VIDEO layers
	 * is less than GRAPHIC layers or more than GRAPHIC layers.
	 * ie, ..., -2, -1, 4, 5, ... (if 0 <= GRAPHIC layer's zpos < 4).
	 * The zpos of VIDEO layers is @b relative. It doesn't need to start
	 * from -1 or 4. Let's suppose that there are two VIDEO layers.
	 * One has -2 zpos. Another has -4 zpos. Then -2 Video layer is higher
	 * than -4 VIDEO layer.
	 */
	tdm_error (*layer_set_video_pos)(tdm_layer *layer, int zpos);

	/**
	 * @brief Create a capture object of a layer object
	 * @param[in] output A output object
	 * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @return A capture object
	 * @see capture_destroy() function of #tdm_func_capture
	 * @remark
	 * A backend module doesn't need to implement this function if a hardware
	 * doesn't have the capture device.
	 */
	tdm_capture *(*layer_create_capture)(tdm_layer *layer, tdm_error *error);

	void (*reserved1)(void);
	void (*reserved2)(void);
	void (*reserved3)(void);
	void (*reserved4)(void);
	void (*reserved5)(void);
	void (*reserved6)(void);
	void (*reserved7)(void);
	void (*reserved8)(void);
} tdm_func_layer;

/**
 * @brief The done handler of a pp object
 */
typedef void (*tdm_pp_done_handler)(tdm_pp *pp, tbm_surface_h src,
                                    tbm_surface_h dst, void *user_data);

/**
 * @brief The pp functions for a backend module.
 */
typedef struct _tdm_func_pp {
	/**
	 * @brief Destroy a pp object
	 * @param[in] pp A pp object
	 * @see display_create_pp() function of #tdm_func_display
	 */
	void         (*pp_destroy)(tdm_pp *pp);

	/**
	 * @brief Set the geometry information to a pp object
	 * @param[in] pp A pp object
	 * @param[in] info The geometry information
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @see pp_commit() function of #tdm_func_pp
	 * @remark
	 * A backend module would apply the geometry information when committed.
	 */
	tdm_error    (*pp_set_info)(tdm_pp *pp, tdm_info_pp *info);

	/**
	 * @brief Attach a source buffer and a destination buffer to a pp object
	 * @param[in] pp A pp object
	 * @param[in] src A source buffer
	 * @param[in] dst A destination buffer
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @see pp_set_info() function of #tdm_func_pp
	 * @see pp_commit() function of #tdm_func_pp
	 * @see pp_set_done_handler, tdm_pp_done_handler
	 * @remark
	 * A backend module converts the image of a source buffer to a destination
	 * buffer when committed. The size/crop/transform information is set via
	 * #pp_set_info() of #tdm_func_pp. When done, a backend module @b SHOULD
	 * return the source/destination buffer via tdm_pp_done_handler.
	 */
	tdm_error    (*pp_attach)(tdm_pp *pp, tbm_surface_h src, tbm_surface_h dst);

	/**
	 * @brief Commit changes for a pp object
	 * @param[in] pp A pp object
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 */
	tdm_error    (*pp_commit)(tdm_pp *pp);

	/**
	 * @brief Set a user done handler to a pp object
	 * @param[in] pp A pp object
	 * @param[in] func A user done handler
	 * @param[in] user_data user data
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @remark
	 * A backend module @b SHOULD call #tdm_pp_done_handler when converintg a image is done.
	 */
	tdm_error    (*pp_set_done_handler)(tdm_pp *pp, tdm_pp_done_handler func,
	                                    void *user_data);

	void (*reserved1)(void);
	void (*reserved2)(void);
	void (*reserved3)(void);
	void (*reserved4)(void);
	void (*reserved5)(void);
	void (*reserved6)(void);
	void (*reserved7)(void);
	void (*reserved8)(void);
} tdm_func_pp;

/**
 * @brief The done handler of a capture object
 */
typedef void (*tdm_capture_done_handler)(tdm_capture *capture,
                tbm_surface_h buffer, void *user_data);

/**
 * @brief The capture functions for a backend module.
 */
typedef struct _tdm_func_capture {
	/**
	 * @brief Destroy a capture object
	 * @param[in] capture A capture object
	 * @see output_create_capture() function of #tdm_func_output
	 * @see layer_create_capture() function of #tdm_func_layer
	 */
	void         (*capture_destroy)(tdm_capture *capture);

	/**
	 * @brief Set the geometry information to a capture object
	 * @param[in] capture A capture object
	 * @param[in] info The geometry information
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @see capture_commit() function of #tdm_func_capture
	 * @remark
	 * A backend module would apply the geometry information when committed.
	 */
	tdm_error    (*capture_set_info)(tdm_capture *capture, tdm_info_capture *info);

	/**
	 * @brief Attach a TDM buffer to a capture object
	 * @details When capture_commit() function is called, a backend module dumps
	 * a output or a layer to a TDM buffer.
	 * @param[in] capture A capture object
	 * @param[in] buffer A TDM buffer
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @see capture_set_info() function of #tdm_func_capture
	 * @see capture_commit() function of #tdm_func_capture
	 * @see capture_set_done_handler, tdm_capture_done_handler
	 * @remark
	 * A backend module dumps a output or a layer to to a TDM buffer when
	 * committed. The size/crop/transform information is set via #capture_set_info()
	 * of #tdm_func_capture. When done, a backend module @b SHOULD return the TDM
	 * buffer via tdm_capture_done_handler.
	 */
	tdm_error    (*capture_attach)(tdm_capture *capture, tbm_surface_h buffer);

	/**
	 * @brief Commit changes for a capture object
	 * @param[in] capture A capture object
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 */
	tdm_error    (*capture_commit)(tdm_capture *capture);

	/**
	 * @brief Set a user done handler to a capture object
	 * @param[in] capture A capture object
	 * @param[in] func A user done handler
	 * @param[in] user_data user data
	 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @remark
	 * A backend module @b SHOULD call #tdm_capture_done_handler when capture operation is done.
	 */
	tdm_error    (*capture_set_done_handler)(tdm_capture *capture,
	                tdm_capture_done_handler func, void *user_data);

	void (*reserved1)(void);
	void (*reserved2)(void);
	void (*reserved3)(void);
	void (*reserved4)(void);
	void (*reserved5)(void);
	void (*reserved6)(void);
	void (*reserved7)(void);
	void (*reserved8)(void);
} tdm_func_capture;

/*
 * ABI versions.  Each version has a major and minor revision.  Modules
 * using lower minor revisions must work with servers of a higher minor
 * revision.  There is no compatibility between different major revisions.
 * Whenever the ABI_ANSIC_VERSION is changed, the others must also be
 * changed.  The minor revision mask is 0x0000FFFF and the major revision
 * mask is 0xFFFF0000.
 */
#define TDM_BACKEND_MINOR_VERSION_MASK  0x0000FFFF
#define TDM_BACKEND_MAJOR_VERSION_MASK  0xFFFF0000
#define TDM_BACKEND_GET_ABI_MINOR(v)    ((v) & TDM_BACKEND_MINOR_VERSION_MASK)
#define TDM_BACKEND_GET_ABI_MAJOR(v)    (((v) & TDM_BACKEND_MAJOR_VERSION_MASK) >> 16)

#define SET_TDM_BACKEND_ABI_VERSION(major, minor) \
        (((major) << 16) & TDM_BACKEND_MAJOR_VERSION_MASK) | \
        ((major) & TDM_BACKEND_MINOR_VERSION_MASK)
#define TDM_BACKEND_ABI_VERSION     SET_TDM_BACKEND_ABI_VERSION(1, 1)

/**
 * @brief The backend module information of the entry point to initialize a TDM
 * backend module.
 * @remark
 * A backend module @b SHOULD define the global data symbol of which name is
 * @b "tdm_backend_module_data". TDM will read this symbol, @b "tdm_backend_module_data",
 * at the initial time and call init() function of #tdm_backend_module.
 */
typedef struct _tdm_backend_module {
	const char *name;           /**< The module name of a backend module */
	const char *vendor;         /**< The vendor name of a backend module */
	unsigned long abi_version;  /**< The ABI version of a backend module */

	/**
	 * @brief The init function of a backend module
	 * @param[in] dpy A display object
	 * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
	 * @return The backend module data
	 * @see tdm_backend_data
	 */
	tdm_backend_data *(*init)(tdm_display *dpy, tdm_error *error);

	/**
	 * @brief The deinit function of a backend module
	 * @param[in] bdata The backend module data
	 */
	void (*deinit)(tdm_backend_data *bdata);
} tdm_backend_module;

/**
 * @brief Register the backend display functions to a display
 * @param[in] dpy A display object
 * @param[in] func_display display functions
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_backend_register_func_output, tdm_backend_register_func_layer
 * @remarks
 * A backend module @b SHOULD set the backend display functions at least.
 */
tdm_error
tdm_backend_register_func_display(tdm_display *dpy,
                                  tdm_func_display *func_display);

/**
 * @brief Register the backend output functions to a display
 * @param[in] dpy A display object
 * @param[in] func_output output functions
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_backend_register_func_display, tdm_backend_register_func_layer
 * @remarks
 * A backend module @b SHOULD set the backend output functions at least.
 */
tdm_error
tdm_backend_register_func_output(tdm_display *dpy,
                                 tdm_func_output *func_output);

/**
 * @brief Register the backend layer functions to a display
 * @param[in] dpy A display object
 * @param[in] func_layer layer functions
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_backend_register_func_display, tdm_backend_register_func_output
 * @remarks
 * A backend module @b SHOULD set the backend layer functions at least.
 */
tdm_error
tdm_backend_register_func_layer(tdm_display *dpy, tdm_func_layer *func_layer);

/**
 * @brief Register the backend pp functions to a display
 * @param[in] dpy A display object
 * @param[in] func_pp pp functions
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_backend_register_func_display, tdm_backend_register_func_capture
 * @remark
 * A backend module doesn'tcan skip to register the backend pp functions
 * if a hardware doesn't have the memory-to-memory converting device.
 */
tdm_error
tdm_backend_register_func_pp(tdm_display *dpy, tdm_func_pp *func_pp);

/**
 * @brief Register the backend capture functions to a display
 * @param[in] dpy A display object
 * @param[in] func_capture capture functions
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_backend_register_func_display, tdm_backend_register_func_pp
 * @remark
 * A backend module can skip to register the backend capture functions
 * if a hardware doesn't have the capture device.
 */
tdm_error
tdm_backend_register_func_capture(tdm_display *dpy,
                                  tdm_func_capture *func_capture);

/**
 * @brief Increase the ref_count of a TDM buffer
 * @details
 * TDM has its own buffer release mechanism to let an frontend user know when a TDM buffer
 * becomes available for a next job. A TDM buffer can be used for TDM to show
 * it on screen or to capture an output and a layer. After all operations,
 * TDM will release it immediately when TDM doesn't need it any more.
 * @param[in] buffer A TDM buffer
 * @return A buffer itself. Otherwise, NULL.
 * @see tdm_buffer_unref_backend
 * @remark
 * - This function @b SHOULD be paired with #tdm_buffer_unref_backend. \n
 * - For example, this function @b SHOULD be called in case that a backend module uses the TDM
 * buffer of a layer for capturing a output or a layer to avoid tearing issue.
 */
tbm_surface_h
tdm_buffer_ref_backend(tbm_surface_h buffer);

/**
 * @brief Decrease the ref_count of a TDM buffer
 * @param[in] buffer A TDM buffer
 * @see tdm_buffer_ref_backend
 */
void
tdm_buffer_unref_backend(tbm_surface_h buffer);

/**
 * @brief The destroy handler of a TDM buffer
 * @param[in] buffer A TDM buffer
 * @param[in] user_data user data
 * @see tdm_buffer_add_destroy_handler, tdm_buffer_remove_destroy_handler
 */
typedef void (*tdm_buffer_destroy_handler)(tbm_surface_h buffer,
                void *user_data);

/**
 * @brief Add a destroy handler to a TDM buffer
 * @param[in] buffer A TDM buffer
 * @param[in] func A destroy handler
 * @param[in] user_data user data
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_buffer_remove_destroy_handler
 * @remark
 * A backend module can add a TDM buffer destroy handler to a TDM buffer which
 * is a #tbm_surface_h object. When the TDM buffer is destroyed, this handler will
 * be called.
 */
tdm_error
tdm_buffer_add_destroy_handler(tbm_surface_h buffer,
                               tdm_buffer_destroy_handler func, void *user_data);

/**
 * @brief Remove a destroy handler from a TDM buffer
 * @param[in] buffer A TDM buffer
 * @param[in] func A destroy handler
 * @param[in] user_data user data
 * @see tdm_buffer_add_destroy_handler
 */
void
tdm_buffer_remove_destroy_handler(tbm_surface_h buffer,
                                  tdm_buffer_destroy_handler func, void *user_data);

/**
 * @brief Add a FD handler for activity on the given file descriptor
 * @param[in] dpy A display object
 * @param[in] fd A file descriptor
 * @param[in] mask to monitor FD
 * @param[in] func A FD handler function
 * @param[in] user_data user data
 * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
 * @return A FD event source
 * @see #tdm_event_loop_source_fd_update, #tdm_event_loop_source_remove
 */
tdm_event_loop_source*
tdm_event_loop_add_fd_handler(tdm_display *dpy, int fd, tdm_event_loop_mask mask,
                         tdm_event_loop_fd_handler func, void *user_data,
                         tdm_error *error);

/**
 * @brief Update the mask of the given FD event source
 * @param[in] source The given FD event source
 * @param[in] mask to monitor FD
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_event_loop_source_fd_update(tdm_event_loop_source *source, tdm_event_loop_mask mask);

/**
 * @brief Add a timer handler
 * @param[in] dpy A display object
 * @param[in] func A timer handler function
 * @param[in] user_data user data
 * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
 * @return A timer event source
 * @see #tdm_event_loop_source_timer_update, #tdm_event_loop_source_remove
 */
tdm_event_loop_source*
tdm_event_loop_add_timer_handler(tdm_display *dpy, tdm_event_loop_timer_handler func,
                            void *user_data, tdm_error *error);

/**
 * @brief Update the millisecond delay time of the given timer event source.
 * @param[in] source The given timer event source
 * @param[in] ms_delay The millisecond delay time. zero "0" disarms the timer.
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_event_loop_source_timer_update(tdm_event_loop_source *source, int ms_delay);

/**
 * @brief Remove the given event source
 * @param[in] source The given event source
 * @see #tdm_event_loop_add_fd_handler, #tdm_event_loop_add_timer_handler
 */
void
tdm_event_loop_source_remove(tdm_event_loop_source *source);

#ifdef __cplusplus
}
#endif

#endif /* _TDM_BACKEND_H_ */
