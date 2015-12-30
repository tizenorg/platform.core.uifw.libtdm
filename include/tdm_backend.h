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
 * @remark It @b SHOULD be freed in the deinit() function of #tdm_backend_module.
 * @see tdm_backend_module, tdm_backend_module
 */
typedef void tdm_backend_data;

/**
 * @brief The display capabilities structure of a backend module
 * @remark A backend module @b SHOULD fill this structure to let frontend know the
 * backend display capabilities.
 * @see The display_get_capabilitiy() function of #tdm_func_display
 */
typedef struct _tdm_caps_display
{
    int max_layer_count;    /**< The maximum layer count. -1 means "not defined" */
} tdm_caps_display;

/**
 * @brief The capabilities structure of a output object
 * @details
 * If a output device has the size restriction, a backend module can let a user
 * know it with min_w, min_h, max_w, max_h, preferred_align variables.
 * @remark A backend module @b SHOULD fill this structure to let frontend know the
 * backend output capabilities.
 * @see The output_get_capability() function of #tdm_func_display
 */
typedef struct _tdm_caps_output
{
    tdm_output_conn_status status;  /**< The connection status */
    tdm_output_type type;           /**< The connection type */
    unsigned int type_id;           /**< The connection type id */

    unsigned int mode_count;        /**< The count of available modes */
    tdm_output_mode *modes;         /**< The array of available modes */

    unsigned int prop_count;        /**< The count of available properties */
    tdm_prop *props;                /**< The array of available properties */

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
 * @remark A backend module @b SHOULD fill this structure to let frontend know the
 * backend layer capabilities.
 * @see The layer_get_capability() function of #tdm_func_display
 */
typedef struct _tdm_caps_layer
{
    tdm_layer_capability capabilities;  /**< The capabilities of layer */

    /**
     * The z-order
     * - The zpos of GRAPHIC layers starts from 0, and it is @b non-changeable.\n
     * - The zpos of VIDEO layers is less than graphic layers or more than graphic layers.
     * ie, -1, -2, 4, 5 (if 0 <= GRAPHIC layer's zpos < 4).. It is @b changeable
     * by layer_set_video_pos() function of #tdm_func_display
    */
    int zpos;

    unsigned int format_count;      /**< The count of available formats */
    tbm_format *formats;            /**< The array of available formats */

    unsigned int prop_count;        /**< The count of available properties */
    tdm_prop *props;                /**< The array of available properties */
} tdm_caps_layer;

/**
 * @brief The capabilities structure of a pp object
 * @details
 * If a pp device has the size restriction, a backend module can let a user
 * know it with min_w, min_h, max_w, max_h, preferred_align variables.
 * @remark A backend module @b SHOULD fill this structure to let frontend know the
 * backend pp capabilities if pp is available.
 * @see The display_get_pp_capability() function of #tdm_func_display
 */
typedef struct _tdm_caps_pp
{
    tdm_pp_capability capabilities; /**< The capabilities of pp */

    unsigned int format_count;      /**< The count of available formats */
    tbm_format *formats;            /**< The array of available formats */

    int min_w;              /**< The minimun width. -1 means "not defined" */
    int min_h;              /**< The minimun height. -1 means "not defined" */
    int max_w;              /**< The maximum width. -1 means "not defined" */
    int max_h;              /**< The maximum height. -1 means "not defined" */
    int preferred_align;    /**< The prefered align. -1 means "not defined" */
} tdm_caps_pp;

/**
 * @brief The capabilities structure of a capture object
 * @details
 * If a capture device has the size restriction, a backend module can let a user
 * know it with min_w, min_h, max_w, max_h, preferred_align variables.
 * @remark A backend module @b SHOULD fill this structure to let frontend know the
 * backend capture capabilities if capture is available.
 * @see The display_get_capture_capability() function of #tdm_func_display
 */
typedef struct _tdm_caps_capture
{
    tdm_capture_capability capabilities;    /**< The capabilities of capture */

    unsigned int format_count;      /**< The count of available formats */
    tbm_format *formats;            /**< The array of available formats */

    int min_w;              /**< The minimun width. -1 means "not defined" */
    int min_h;              /**< The minimun height. -1 means "not defined" */
    int max_w;              /**< The maximum width. -1 means "not defined" */
    int max_h;              /**< The maximum height. -1 means "not defined" */
    int preferred_align;    /**< The prefered align. -1 means "not defined" */
} tdm_caps_capture;

/**
 * @brief The display functions for a backend module.
 */
typedef struct _tdm_func_display
{
    /**
     * @brief Get the display capabilities of a backend module
     * @details TDM calls this function at the initial time. And at the update time also.
     * #tdm_caps_display containes the max layer count information, etc.
     * @param[in] bdata The backend module data
     * @param[out] caps The display capabilities of a backend module
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     */
    tdm_error    (*display_get_capabilitiy)(tdm_backend_data *bdata, tdm_caps_display *caps);

    /**
     * @brief Get the pp capabilities of a backend module
     * @details This function can be NULL if a backend module doesn't have the pp capability.
     * TDM calls this function at the initial time if available. And at the update time also.
     * #tdm_caps_pp containes avaiable formats, size restriction information, etc.
     * @param[in] bdata The backend module data
     * @param[out] caps The pp capabilities of a backend module
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     * @see tdm_backend_register_func_pp
     */
    tdm_error    (*display_get_pp_capability)(tdm_backend_data *bdata, tdm_caps_pp *caps);

    /**
     * @brief Get the capture capabilities of a backend module
     * @details This function can be NULL if a backend module doesn't have the capture capability.
     * TDM calls this function at the initial time if available. And at the update time also.
     * #tdm_caps_capture containes avaiable formats, size restriction information, etc.
     * @param[in] bdata The backend module data
     * @param[out] caps The capture capabilities of a backend module
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     * @see tdm_backend_register_func_capture
     */
    tdm_error    (*display_get_capture_capability)(tdm_backend_data *bdata, tdm_caps_capture *caps);

    /**
     * @brief Get a output array of a backend module
     * @details A backend module @b SHOULD return the newly-allocated output array.
     * it will be freed in frontend.
     * @param[in] bdata The backend module data
     * @param[out] count The count of outputs
     * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
     * @return A output array which is newly-allocated
     * @see tdm_backend_register_func_capture
     */
    tdm_output **(*display_get_outputs)(tdm_backend_data *bdata, int *count, tdm_error *error);

    /**
     * @brief Get the file descriptor of a backend module
     * @details If a backend module handles one more fds, a backend module can return epoll's fd
     * which is watching backend device fds.
     * @param[in] bdata The backend module data
     * @param[out] fd The fd of a backend module
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     * @see display_handle_events() function of #tdm_func_display
     */
    tdm_error    (*display_get_fd)(tdm_backend_data *bdata, int *fd);

    /**
     * @brief Handle the events which happen on the fd of a backend module
     * @param[in] bdata The backend module data
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     */
    tdm_error    (*display_handle_events)(tdm_backend_data *bdata);

    /**
     * @brief Create a pp object of a backend module
     * @param[in] bdata The backend module data
     * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
     * @return A pp object
     * @see pp_destroy() function of #tdm_func_pp
     */
    tdm_pp*      (*display_create_pp)(tdm_backend_data *bdata, tdm_error *error);

    /**
     * @brief Get the capabilities of a output object
     * #tdm_caps_output containes connection, modes, avaiable properties, size restriction information, etc.
     * @param[in] output A output object
     * @param[out] caps The capabilities of a output object
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     */
    tdm_error    (*output_get_capability)(tdm_output *output, tdm_caps_output *caps);

    /**
     * @brief Get a layer array of a output object
     * @details A backend module @b SHOULD return the newly-allocated layer array.
     * it will be freed in frontend.
     * @param[in] output A output object
     * @param[out] count The count of layers
     * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
     * @return A layer array which is newly-allocated
     */
    tdm_layer  **(*output_get_layers)(tdm_output *output, int *count, tdm_error *error);

    /**
     * @brief Set the property which has a given id
     * @param[in] output A output object
     * @param[in] id The property id
     * @param[in] value The value
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     */
    tdm_error    (*output_set_property)(tdm_output *output, unsigned int id, tdm_value value);

    /**
     * @brief Get the property which has a given id
     * @param[in] output A output object
     * @param[in] id The property id
     * @param[out] value The value
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     */
    tdm_error    (*output_get_property)(tdm_output *output, unsigned int id, tdm_value *value);

    /**
     * @brief Wait for VBLANK
     * @remark After interval vblanks, a backend module @b SHOULD call a user
     * vblank handler.
     * @param[in] output A output object
     * @param[in] interval vblank interval
     * @param[in] sync 0: asynchronous, 1:synchronous
     * @param[in] user_data The user data
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     * @see #tdm_output_vblank_handler
     */
    tdm_error    (*output_wait_vblank)(tdm_output *output, int interval, int sync, void *user_data);

    /**
     * @brief Set a user vblank handler
     * @param[in] output A output object
     * @param[in] func A user vblank handler
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     */
    tdm_error    (*output_set_vblank_handler)(tdm_output *output, tdm_output_vblank_handler func);

    /**
     * @brief Commit changes for a output object
     * @remark After all change of a output object are applied, a backend module
     * @b SHOULD call a user commit handler.
     * @param[in] output A output object
     * @param[in] sync 0: asynchronous, 1:synchronous
     * @param[in] user_data The user data
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     */
    tdm_error    (*output_commit)(tdm_output *output, int sync, void *user_data);

    /**
     * @brief Set a user commit handler
     * @param[in] output A output object
     * @param[in] func A user commit handler
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     */
    tdm_error    (*output_set_commit_handler)(tdm_output *output, tdm_output_commit_handler func);

    /**
     * @brief Set DPMS of a output object
     * @param[in] output A output object
     * @param[in] dpms_value DPMS value
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     */
    tdm_error    (*output_set_dpms)(tdm_output *output, tdm_output_dpms dpms_value);

    /**
     * @brief Get DPMS of a output object
     * @param[in] output A output object
     * @param[out] dpms_value DPMS value
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     */
    tdm_error    (*output_get_dpms)(tdm_output *output, tdm_output_dpms *dpms_value);

    /**
     * @brief Set one of available modes of a output object
     * @param[in] output A output object
     * @param[in] mode A output mode
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     */
    tdm_error    (*output_set_mode)(tdm_output *output, const tdm_output_mode *mode);

    /**
     * @brief Get the mode of a output object
     * @param[in] output A output object
     * @param[out] mode A output mode
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     */
    tdm_error    (*output_get_mode)(tdm_output *output, const tdm_output_mode **mode);

    /**
     * @brief Create a capture object of a output object
     * @param[in] output A output object
     * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
     * @return A capture object
     * @see capture_destroy() function of #tdm_func_capture
     */
    tdm_capture *(*output_create_capture)(tdm_output *output, tdm_error *error);

    /**
     * @brief Get the capabilities of a layer object
     * #tdm_caps_layer containes avaiable formats/properties, zpos information, etc.
     * @param[in] layer A layer object
     * @param[out] caps The capabilities of a layer object
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     */
    tdm_error    (*layer_get_capability)(tdm_layer *layer, tdm_caps_layer *caps);

    /**
     * @brief Set the property which has a given id.
     * @param[in] layer A layer object
     * @param[in] id The property id
     * @param[in] value The value
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     */
    tdm_error    (*layer_set_property)(tdm_layer *layer, unsigned int id, tdm_value value);

    /**
     * @brief Get the property which has a given id.
     * @param[in] layer A layer object
     * @param[in] id The property id
     * @param[out] value The value
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     */
    tdm_error    (*layer_get_property)(tdm_layer *layer, unsigned int id, tdm_value *value);

    /**
     * @brief Set the geometry information to a layer object
     * @details The geometry information will be applied when the output object
     * of a layer object is committed.
     * @param[in] layer A layer object
     * @param[in] info The geometry information
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     * @see output_commit() function of #tdm_func_display
     */
    tdm_error    (*layer_set_info)(tdm_layer *layer, tdm_info_layer *info);

    /**
     * @brief Get the geometry information to a layer object
     * @param[in] layer A layer object
     * @param[out] info The geometry information
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     */
    tdm_error    (*layer_get_info)(tdm_layer *layer, tdm_info_layer *info);

    /**
     * @brief Set a TDM buffer to a layer object
     * @details A TDM buffer will be applied when the output object
     * of a layer object is committed.
     * @param[in] layer A layer object
     * @param[in] buffer A TDM buffer
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     * @see output_commit() function of #tdm_func_display
     */
    tdm_error    (*layer_set_buffer)(tdm_layer *layer, tbm_surface_h buffer);

    /**
     * @brief Unset a TDM buffer from a layer object
     * @details When this function is called, a current showing buffer will be
     * disappeared from screen. Then nothing is showing on a layer object.
     * @param[in] layer A layer object
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     */
    tdm_error    (*layer_unset_buffer)(tdm_layer *layer);

    /**
     * @brief Set the zpos for a VIDEO layer object
     * @details Especially this function is for a VIDEO layer. The zpos of
     * GRAPHIC layers can't be changed.
     * @param[in] layer A layer object
     * @param[in] zpos z-order
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     * @see tdm_layer_capability
     */
    tdm_error    (*layer_set_video_pos)(tdm_layer *layer, int zpos);

    /**
     * @brief Create a capture object of a layer object
     * @param[in] output A output object
     * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
     * @return A capture object
     * @see capture_destroy() function of #tdm_func_capture
     */
    tdm_capture *(*layer_create_capture)(tdm_layer *layer, tdm_error *error);
} tdm_func_display;

/**
 * @brief The done handler of a pp object
 */
typedef void (*tdm_pp_done_handler)(tdm_pp *pp, tbm_surface_h src, tbm_surface_h dst, void *user_data);

/**
 * @brief The pp functions for a backend module.
 */
typedef struct _tdm_func_pp
{
    /**
     * @brief Destroy a pp object
     * @param[in] pp A pp object
     * @see display_create_pp() function of #tdm_func_display
     */
    void         (*pp_destroy)(tdm_pp *pp);

    /**
     * @brief Set the geometry information to a pp object
     * @details The geometry information will be applied when the pp object is committed.
     * @param[in] pp A pp object
     * @param[in] info The geometry information
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     * @see pp_commit() function of #tdm_func_pp
     */
    tdm_error    (*pp_set_info)(tdm_pp *pp, tdm_info_pp *info);

    /**
     * @brief Attach a source buffer and a destination buffer to a pp object
     * @details When pp_commit() function is called, a backend module converts
     * the image of a source buffer to a destination buffer.
     * @param[in] pp A pp object
     * @param[in] src A source buffer
     * @param[in] dst A destination buffer
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     * @see tdm_pp_done_handler
     * @see pp_set_info() function of #tdm_func_pp
     * @see pp_commit() function of #tdm_func_pp
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
     * @remark When pp operation is done, a backend module @b SHOULD call #tdm_pp_done_handler.
     */
    tdm_error    (*pp_set_done_handler)(tdm_pp *pp, tdm_pp_done_handler func, void *user_data);
} tdm_func_pp;

/**
 * @brief The done handler of a capture object
 */
typedef void (*tdm_capture_done_handler)(tdm_capture *capture, tbm_surface_h buffer, void *user_data);

/**
 * @brief The capture functions for a backend module.
 */
typedef struct _tdm_func_capture
{
    /**
     * @brief Destroy a capture object
     * @param[in] capture A capture object
     * @see output_create_capture() function of #tdm_func_display
     * @see layer_create_capture() function of #tdm_func_display
     */
    void         (*capture_destroy)(tdm_capture *capture);

    /**
     * @brief Set the geometry information to a capture object
     * @details The geometry information will be applied when the capture object is committed.
     * @param[in] capture A capture object
     * @param[in] info The geometry information
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     * @see capture_commit() function of #tdm_func_capture
     */
    tdm_error    (*capture_set_info)(tdm_capture *capture, tdm_info_capture *info);

    /**
     * @brief Attach a TDM buffer to a capture object
     * @details When capture_commit() function is called, a backend module dumps
     * a output or a layer to a TDM buffer.
     * @param[in] capture A capture object
     * @param[in] buffer A TDM buffer
     * @return #TDM_ERROR_NONE if success. Otherwise, error value.
     * @see tdm_capture_done_handler
     * @see capture_commit() function of #tdm_func_capture
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
     * @remark When capture operation is done, a backend module @b SHOULD call #tdm_capture_done_handler.
     */
    tdm_error    (*capture_set_done_handler)(tdm_capture *capture, tdm_capture_done_handler func, void *user_data);
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
 * @details
 * A backend module @b SHOULD define the global data symbol of which name is
 * @b "tdm_backend_module_data". TDM will read this symbol, @b "tdm_backend_module_data",
 * at the initial time and call init() function of #tdm_backend_module.
 */
typedef struct _tdm_backend_module
{
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
    tdm_backend_data* (*init)(tdm_display *dpy, tdm_error *error);

    /**
     * @brief The deinit function of a backend module
     * @param[in] bdata The backend module data
     */
    void (*deinit)(tdm_backend_data *bdata);
} tdm_backend_module;

/**
 * @brief Register the backend display functions to a display
 * @details
 * A backend module @b SHOULD set the backend display functions at least.
 * @param[in] dpy A display object
 * @param[in] func_display display functions
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_backend_register_func_pp, tdm_backend_register_func_capture
 */
tdm_error tdm_backend_register_func_display(tdm_display *dpy, tdm_func_display *func_display);

/**
 * @brief Register the backend pp functions to a display
 * @details
 * A backend module can skip to register the backend pp functions
 * if hardware device doesn't supports the memory to memory converting.
 * @param[in] dpy A display object
 * @param[in] func_pp pp functions
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_backend_register_func_display, tdm_backend_register_func_capture
 */
tdm_error tdm_backend_register_func_pp(tdm_display *dpy, tdm_func_pp *func_pp);

/**
 * @brief Register the backend capture functions to a display
 * @details
 * A backend module can skip to register the backend capture functions
 * if hardware device doesn't supports the memory to memory converting.
 * @param[in] dpy A display object
 * @param[in] func_capture capture functions
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_backend_register_func_display, tdm_backend_register_func_pp
 */
tdm_error tdm_backend_register_func_capture(tdm_display *dpy, tdm_func_capture *func_capture);

/**
 * @brief Increase the ref_count of a TDM buffer
 * @details
 * TDM has its own buffer release mechanism to let an frontend user know when a TDM buffer
 * becomes available for a next job. A TDM buffer can be used for TDM to show
 * it on screen or to capture an output and a layer. After all operations,
 * TDM will release it immediately when TDM doesn't need it any more.
 * @param[in] buffer A TDM buffer
 * @return A buffer itself. Otherwise, NULL.
 * @remark
 * - This function @b SHOULD be paired with #tdm_buffer_unref_backend. \n
 * - For example, this function @b SHOULD be called in case that a backend module uses the TDM
 * buffer of a layer for capturing a output or a layer to avoid tearing issue.
 * @see tdm_buffer_unref_backend
 */
tbm_surface_h tdm_buffer_ref_backend(tbm_surface_h buffer);

/**
 * @brief Decrease the ref_count of a TDM buffer
 * @param[in] buffer A TDM buffer
 * @see tdm_buffer_ref_backend
 */
void          tdm_buffer_unref_backend(tbm_surface_h buffer);

/**
 * @brief The destroy handler of a TDM buffer
 * @param[in] buffer A TDM buffer
 * @param[in] user_data user data
 * @see tdm_buffer_add_destroy_handler, tdm_buffer_remove_destroy_handler
 */
typedef void (*tdm_buffer_destroy_handler)(tbm_surface_h buffer, void *user_data);

/**
 * @brief Add a destroy handler to a TDM buffer
 * @details
 * A backend module can add a TDM buffer destroy handler to a TDM buffer which
 * is a #tbm_surface_h object. When the TDM buffer is destroyed, this handler will
 * be called.
 * @param[in] buffer A TDM buffer
 * @param[in] func A destroy handler
 * @param[in] user_data user data
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see tdm_buffer_remove_destroy_handler
 */
tdm_error     tdm_buffer_add_destroy_handler(tbm_surface_h buffer, tdm_buffer_destroy_handler func, void *user_data);

/**
 * @brief Remove a destroy handler from a TDM buffer
 * @param[in] buffer A TDM buffer
 * @param[in] func A destroy handler
 * @param[in] user_data user data
 * @see tdm_buffer_add_destroy_handler
 */
void          tdm_buffer_remove_destroy_handler(tbm_surface_h buffer, tdm_buffer_destroy_handler func, void *user_data);


#ifdef __cplusplus
}
#endif

#endif /* _TDM_BACKEND_H_ */
