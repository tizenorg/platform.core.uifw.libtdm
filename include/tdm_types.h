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

#ifndef _TDM_TYPES_H_
#define _TDM_TYPES_H_

#include <tbm_surface.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TDM_NAME_LEN        64

/**
 * @file tdm_types.h
 * @brief The header file which defines Enumerations and Structures for frontend and backend.
 * @details
 * Both frontend(@ref tdm.h) and backend(@ref tdm_backend.h) header files
 * include @ref tdm_types.h
 * @par Example
 * @code
   #include <tdm.h>    //for a frontend user
 * @endcode
 * @code
   #include <tdm_backend.h>  //for a vendor to implement a backend module
 * @endcode
 */

/**
 * @brief The error enumeration
 */
typedef enum
{
	TDM_ERROR_NONE                  = 0,  /**< none */
	TDM_ERROR_BAD_REQUEST           = -1, /**< bad request */
	TDM_ERROR_OPERATION_FAILED      = -2, /**< operaion failed */
	TDM_ERROR_INVALID_PARAMETER     = -3, /**< wrong input parameter */
	TDM_ERROR_PERMISSION_DENIED     = -4, /**< access denied */
	TDM_ERROR_BUSY                  = -5, /**< hardware resource busy */
	TDM_ERROR_OUT_OF_MEMORY         = -6, /**< no free memory */
	TDM_ERROR_BAD_MODULE            = -7, /**< bad backend module */
	TDM_ERROR_NOT_IMPLEMENTED       = -8, /**< not implemented */
	TDM_ERROR_NO_CAPABILITY         = -9, /**< no capability */
} tdm_error;

/**
 * @brief The transform enumeration(rotate, flip)
 */
typedef enum {
	TDM_TRANSFORM_NORMAL            = 0, /**< no transform */
	TDM_TRANSFORM_90                = 1, /**< rotate 90 */
	TDM_TRANSFORM_180               = 2, /**< rotate 180 */
	TDM_TRANSFORM_270               = 3, /**< rotate 270 */
	TDM_TRANSFORM_FLIPPED           = 4, /**< no rotate and horizontal flip */
	TDM_TRANSFORM_FLIPPED_90        = 5, /**< rotate 90 and horizontal flip */
	TDM_TRANSFORM_FLIPPED_180       = 6, /**< rotate 180 and horizontal flip */
	TDM_TRANSFORM_FLIPPED_270       = 7, /**< rotate 270 and horizontal flip */
} tdm_transform;

/**
 * @brief The output connection status enumeration
 */
typedef enum {
	TDM_OUTPUT_CONN_STATUS_DISCONNECTED, /**< output disconnected */
	TDM_OUTPUT_CONN_STATUS_CONNECTED,    /**< output connected */
	TDM_OUTPUT_CONN_STATUS_MODE_SETTED,  /**< output connected and setted a mode */
} tdm_output_conn_status;

/**
 * @brief The output connection status enumeration
 * @details bit compatible with the libdrm definitions.
 */
typedef enum {
	TDM_OUTPUT_TYPE_Unknown,        /**< unknown */
	TDM_OUTPUT_TYPE_VGA,            /**< VGA connection */
	TDM_OUTPUT_TYPE_DVII,           /**< DVII connection */
	TDM_OUTPUT_TYPE_DVID,           /**< DVID connection */
	TDM_OUTPUT_TYPE_DVIA,           /**< DVIA connection */
	TDM_OUTPUT_TYPE_Composite,      /**< Composite connection */
	TDM_OUTPUT_TYPE_SVIDEO,         /**< SVIDEO connection */
	TDM_OUTPUT_TYPE_LVDS,           /**< LVDS connection */
	TDM_OUTPUT_TYPE_Component,      /**< Component connection */
	TDM_OUTPUT_TYPE_9PinDIN,        /**< 9PinDIN connection */
	TDM_OUTPUT_TYPE_DisplayPort,    /**< DisplayPort connection */
	TDM_OUTPUT_TYPE_HDMIA,          /**< HDMIA connection */
	TDM_OUTPUT_TYPE_HDMIB,          /**< HDMIB connection */
	TDM_OUTPUT_TYPE_TV,             /**< TV connection */
	TDM_OUTPUT_TYPE_eDP,            /**< eDP connection */
	TDM_OUTPUT_TYPE_VIRTUAL,        /**< Virtual connection for WiFi Display */
	TDM_OUTPUT_TYPE_DSI,            /**< DSI connection */
} tdm_output_type;

/**
 * @brief The DPMS enumeration
 * @details bit compatible with the libdrm definitions.
 */
typedef enum {
	TDM_OUTPUT_DPMS_ON,         /**< On */
	TDM_OUTPUT_DPMS_STANDBY,    /**< StandBy */
	TDM_OUTPUT_DPMS_SUSPEND,    /**< Suspend */
	TDM_OUTPUT_DPMS_OFF,        /**< Off */
} tdm_output_dpms;

/**
 * @brief The layer capability enumeration
 * @details
 * A layer can have one of CURSOR, PRIMARY and OVERLAY capability. And a layer
 * also can have one of GRAPHIC and VIDEO capability. And a layer also can have
 * SCALE and TRANSFORM capability.\n
 * @par Example
 * @code
   //For example
   capabilities = TDM_LAYER_CAPABILITY_PRIMARY | TDM_LAYER_CAPABILITY_GRAPHIC;
   capabilities = TDM_LAYER_CAPABILITY_OVERLAY | TDM_LAYER_CAPABILITY_GRAPHIC | TDM_LAYER_CAPABILITY_SCALE;
   capabilities = TDM_LAYER_CAPABILITY_OVERLAY | TDM_LAYER_CAPABILITY_GRAPHIC | TDM_LAYER_CAPABILITY_SCALE | TDM_LAYER_CAPABILITY_TRANSFORM;
   capabilities = TDM_LAYER_CAPABILITY_CURSOR | TDM_LAYER_CAPABILITY_GRAPHIC;
   capabilities = TDM_LAYER_CAPABILITY_OVERLAY | TDM_LAYER_CAPABILITY_VIDEO;
 * @endcode
 * @remark
 * - When a video plays, in most of cases, video buffers will be displayed to
 * a GRAPHIC layer after converting RGB buffers via PP. In this case, a backend
 * module doesn't need to offer VIDEO layer.
 * - But in case that s vendor wants to handle a video by their own way,
 * a backend module offer VIDEO layers. And a display server will pass a video
 * buffer to a VIDEO layer without converting.
 */
typedef enum {
	TDM_LAYER_CAPABILITY_CURSOR         = (1 << 0), /**< cursor */
	TDM_LAYER_CAPABILITY_PRIMARY        = (1 << 1), /**< primary */
	TDM_LAYER_CAPABILITY_OVERLAY        = (1 << 2), /**< overlay */
	TDM_LAYER_CAPABILITY_GRAPHIC        = (1 << 4), /**< graphic */
	TDM_LAYER_CAPABILITY_VIDEO          = (1 << 5), /**< video */
	TDM_LAYER_CAPABILITY_SCALE          = (1 << 8), /**< if a layer has scale capability  */
	TDM_LAYER_CAPABILITY_TRANSFORM      = (1 << 9), /**< if a layer has transform capability  */
	TDM_LAYER_CAPABILITY_SCANOUT        = (1 << 10), /**< if a layer allows a scanout buffer only */
	TDM_LAYER_CAPABILITY_RESEVED_MEMORY = (1 << 11), /**< if a layer allows a reserved buffer only */
	TDM_LAYER_CAPABILITY_NO_CROP        = (1 << 12), /**< if a layer has no cropping capability */
} tdm_layer_capability;

/**
 * @brief The pp capability enumeration
 */
typedef enum {
	TDM_PP_CAPABILITY_SYNC           = (1 << 0), /**< The pp device supports synchronous operation */
	TDM_PP_CAPABILITY_ASYNC          = (1 << 1), /**< The pp device supports asynchronous operation */
	TDM_PP_CAPABILITY_SCALE          = (1 << 4), /**< The pp device supports scale operation */
	TDM_PP_CAPABILITY_TRANSFORM      = (1 << 5), /**< The pp device supports transform operation */
} tdm_pp_capability;

/**
 * @brief The capture capability enumeration
 */
typedef enum {
	TDM_CAPTURE_CAPABILITY_OUTPUT    = (1 << 0), /**< The capture device supports to dump a output */
	TDM_CAPTURE_CAPABILITY_LAYER     = (1 << 1), /**< The capture device supports to dump a layer */
	TDM_CAPTURE_CAPABILITY_SCALE     = (1 << 4), /**< The capture device supports scale operation */
	TDM_CAPTURE_CAPABILITY_TRANSFORM = (1 << 5), /**< The capture device supports transform operation */
} tdm_capture_capability;

/**
 * @brief The output mode type enumeration
 * @details bit compatible with the libdrm definitions.
 */
typedef enum {
	TDM_OUTPUT_MODE_TYPE_BUILTIN    = (1 << 0),
	TDM_OUTPUT_MODE_TYPE_CLOCK_C    = ((1 << 1) | TDM_OUTPUT_MODE_TYPE_BUILTIN),
	TDM_OUTPUT_MODE_TYPE_CRTC_C     = ((1 << 2) | TDM_OUTPUT_MODE_TYPE_BUILTIN),
	TDM_OUTPUT_MODE_TYPE_PREFERRED  = (1 << 3),
	TDM_OUTPUT_MODE_TYPE_DEFAULT    = (1 << 4),
	TDM_OUTPUT_MODE_TYPE_USERDEF    = (1 << 5),
	TDM_OUTPUT_MODE_TYPE_DRIVER     = (1 << 6),
} tdm_output_mode_type;

/**
 * @brief The output mode flag enumeration
 * @details bit compatible with the libdrm definitions.
 */
typedef enum {
	TDM_OUTPUT_MODE_FLAG_PHSYNC     = (1 << 0),
	TDM_OUTPUT_MODE_FLAG_NHSYNC     = (1 << 1),
	TDM_OUTPUT_MODE_FLAG_PVSYNC     = (1 << 2),
	TDM_OUTPUT_MODE_FLAG_NVSYNC     = (1 << 3),
	TDM_OUTPUT_MODE_FLAG_INTERLACE  = (1 << 4),
	TDM_OUTPUT_MODE_FLAG_DBLSCAN    = (1 << 5),
	TDM_OUTPUT_MODE_FLAG_CSYNC      = (1 << 6),
	TDM_OUTPUT_MODE_FLAG_PCSYNC     = (1 << 7),
	TDM_OUTPUT_MODE_FLAG_NCSYNC     = (1 << 8),
	TDM_OUTPUT_MODE_FLAG_HSKEW      = (1 << 9), /* hskew provided */
	TDM_OUTPUT_MODE_FLAG_BCAST      = (1 << 10),
	TDM_OUTPUT_MODE_FLAG_PIXMUX     = (1 << 11),
	TDM_OUTPUT_MODE_FLAG_DBLCLK     = (1 << 12),
	TDM_OUTPUT_MODE_FLAG_CLKDIV2    = (1 << 13),
} tdm_output_mode_flag;

typedef enum
{
	TDM_EVENT_LOOP_READABLE = (1 << 0),
	TDM_EVENT_LOOP_WRITABLE = (1 << 1),
	TDM_EVENT_LOOP_HANGUP   = (1 << 2),
	TDM_EVENT_LOOP_ERROR    = (1 << 3),
} tdm_event_loop_mask;

/**
 * @brief The output mode structure
 */
typedef struct _tdm_output_mode {
	unsigned int clock;
	unsigned int hdisplay, hsync_start, hsync_end, htotal, hskew;
	unsigned int vdisplay, vsync_start, vsync_end, vtotal, vscan;
	unsigned int vrefresh;
	unsigned int flags;
	unsigned int type;
	char name[TDM_NAME_LEN];
} tdm_output_mode;

/**
 * @brief The property structure
 */
typedef struct _tdm_prop {
	unsigned int id;
	char name[TDM_NAME_LEN];
} tdm_prop;

/**
 * @brief The size structure
 */
typedef struct _tdm_size {
	unsigned int h;     /**< width */
	unsigned int v;     /**< height */
} tdm_size;

/**
 * @brief The pos structure
 */
typedef struct _tdm_pos {
	unsigned int x;
	unsigned int y;
	unsigned int w;
	unsigned int h;
} tdm_pos;

/**
 * @brief The value union
 */
typedef union {
	void	 *ptr;
	int32_t  s32;
	uint32_t u32;
	int64_t  s64;
	uint64_t u64;
} tdm_value;

/**
 * @brief The info config structure
 */
typedef struct _tdm_info_config {
	tdm_size size;
	tdm_pos pos;
	tbm_format format;
} tdm_info_config;

/**
 * @brief The layer info structre
 */
typedef struct _tdm_info_layer {
	tdm_info_config src_config;
	tdm_pos dst_pos;
	tdm_transform transform;
} tdm_info_layer;

/**
 * @brief The pp info structre
 */
typedef struct _tdm_info_pp {
	tdm_info_config src_config;
	tdm_info_config dst_config;
	tdm_transform transform;
	int sync;
	int flags;
} tdm_info_pp;

/**
 * @brief The capture info structre
 */
typedef struct _tdm_info_capture {
	tdm_info_config dst_config;
	tdm_transform transform;
	int oneshot;
	int frequency;
	int flags;
} tdm_info_capture;

/**
 * @brief The tdm display object
 */
typedef void tdm_display;

/**
 * @brief The tdm output object
 */
typedef void tdm_output;

/**
 * @brief The tdm layer object
 */
typedef void tdm_layer;

/**
 * @brief The tdm capture object
 */
typedef void tdm_capture;

/**
 * @brief The tdm pp object
 */
typedef void tdm_pp;

/**
 * @brief The vblank handler
 * @see output_set_vblank_handler() function of #tdm_func_display
 */
typedef void (*tdm_output_vblank_handler)(tdm_output *output, unsigned int sequence,
                                          unsigned int tv_sec, unsigned int tv_usec,
                                          void *user_data);

/**
 * @brief The commit handler
 * @see output_set_commit_handler() function of #tdm_func_display
 */
typedef void (*tdm_output_commit_handler)(tdm_output *output, unsigned int sequence,
                                          unsigned int tv_sec, unsigned int tv_usec,
                                          void *user_data);

/**
 * @brief The tdm event source
 */
typedef void tdm_event_loop_source;

/**
 * @brief The fd source handler
 */
typedef tdm_error (*tdm_event_loop_fd_handler)(int fd, tdm_event_loop_mask mask, void *user_data);

/**
 * @brief The timer source handler
 */
typedef tdm_error (*tdm_event_loop_timer_handler)(void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* _TDM_TYPES_H_ */
