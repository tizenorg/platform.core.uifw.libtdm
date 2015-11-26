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

typedef enum
{
    TDM_ERROR_NONE                  = 0,
    TDM_ERROR_BAD_REQUEST           = -1, //bad request
    TDM_ERROR_OPERATION_FAILED      = -2, //operaion failed
    TDM_ERROR_INVALID_PARAMETER     = -3, //wrong input parameter
    TDM_ERROR_PERMISSION_DENIED     = -4, //access denied
    TDM_ERROR_BUSY                  = -5, //hardware resource busy
    TDM_ERROR_OUT_OF_MEMORY         = -6, //no free memory
    TDM_ERROR_BAD_MODULE            = -7, //bad backend module
    TDM_ERROR_NOT_IMPLEMENTED       = -8, //not implemented
} tdm_error;

typedef enum
{
    TDM_TRANSFORM_NORMAL            = 0,
    TDM_TRANSFORM_90                = 1,
    TDM_TRANSFORM_180               = 2,
    TDM_TRANSFORM_270               = 3,
    TDM_TRANSFORM_FLIPPED           = 4,
    TDM_TRANSFORM_FLIPPED_90        = 5,
    TDM_TRANSFORM_FLIPPED_180       = 6,
    TDM_TRANSFORM_FLIPPED_270       = 7,
    TDM_TRANSFORM_MAX               = 8
} tdm_transform;

typedef enum
{
    TDM_OUTPUT_CONN_STATUS_DISCONNECTED,
    TDM_OUTPUT_CONN_STATUS_CONNECTED,
    TDM_OUTPUT_CONN_STATUS_MODE_SETTED,
    TDM_OUTPUT_CONN_STATUS_MAX,
} tdm_output_conn_status;

/* bit compatible with the libdrm definitions. */
typedef enum
{
    TDM_OUTPUT_TYPE_Unknown,
    TDM_OUTPUT_TYPE_VGA,
    TDM_OUTPUT_TYPE_DVII,
    TDM_OUTPUT_TYPE_DVID,
    TDM_OUTPUT_TYPE_DVIA,
    TDM_OUTPUT_TYPE_Composite,
    TDM_OUTPUT_TYPE_SVIDEO,
    TDM_OUTPUT_TYPE_LVDS,
    TDM_OUTPUT_TYPE_Component,
    TDM_OUTPUT_TYPE_9PinDIN,
    TDM_OUTPUT_TYPE_DisplayPort,
    TDM_OUTPUT_TYPE_HDMIA,
    TDM_OUTPUT_TYPE_HDMIB,
    TDM_OUTPUT_TYPE_TV,
    TDM_OUTPUT_TYPE_eDP,
    TDM_OUTPUT_TYPE_VIRTUAL,
    TDM_OUTPUT_TYPE_DSI,
    TDM_OUTPUT_TYPE_MAX,
} tdm_output_type;

/* bit compatible with the drm definitions. */
typedef enum
{
    TDM_OUTPUT_DPMS_ON,
    TDM_OUTPUT_DPMS_STANDBY,
    TDM_OUTPUT_DPMS_SUSPEND,
    TDM_OUTPUT_DPMS_OFF,
    TDM_OUTPUT_DPMS_MAX,
} tdm_output_dpms;

typedef enum
{
    TDM_DISPLAY_CAPABILITY_PP       = (1<<0),
    TDM_DISPLAY_CAPABILITY_CAPTURE  = (1<<1),
} tdm_display_capability;

typedef enum
{
    TDM_LAYER_CAPABILITY_CURSOR         = (1<<0),
    TDM_LAYER_CAPABILITY_PRIMARY        = (1<<1),
    TDM_LAYER_CAPABILITY_OVERLAY        = (1<<2),
    TDM_LAYER_CAPABILITY_SCALE          = (1<<4),
    TDM_LAYER_CAPABILITY_TRANSFORM      = (1<<5),
    TDM_LAYER_CAPABILITY_GRAPHIC        = (1<<8),
    TDM_LAYER_CAPABILITY_VIDEO          = (1<<9),
} tdm_layer_capability;

typedef enum
{
    TDM_PP_CAPABILITY_SYNC           = (1<<0),
    TDM_PP_CAPABILITY_ASYNC          = (1<<1),
    TDM_PP_CAPABILITY_SCALE          = (1<<4),
    TDM_PP_CAPABILITY_TRANSFORM      = (1<<5),
} tdm_pp_capability;

typedef enum
{
    TDM_CAPTURE_CAPABILITY_OUTPUT    = (1<<0),
    TDM_CAPTURE_CAPABILITY_LAYER     = (1<<1),
    TDM_CAPTURE_CAPABILITY_SCALE     = (1<<4),
    TDM_CAPTURE_CAPABILITY_TRANSFORM = (1<<5),
} tdm_capture_capability;

/* bit compatible with the drm definitions. */
typedef enum
{
    TDM_OUTPUT_MODE_TYPE_BUILTIN    = (1<<0),
    TDM_OUTPUT_MODE_TYPE_CLOCK_C    = ((1<<1) | TDM_OUTPUT_MODE_TYPE_BUILTIN),
    TDM_OUTPUT_MODE_TYPE_CRTC_C     = ((1<<2) | TDM_OUTPUT_MODE_TYPE_BUILTIN),
    TDM_OUTPUT_MODE_TYPE_PREFERRED  = (1<<3),
    TDM_OUTPUT_MODE_TYPE_DEFAULT    = (1<<4),
    TDM_OUTPUT_MODE_TYPE_USERDEF    = (1<<5),
    TDM_OUTPUT_MODE_TYPE_DRIVER     = (1<<6),
} tdm_output_mode_type;

/* bit compatible with the drm definitions. */
typedef enum
{
    TDM_OUTPUT_MODE_FLAG_PHSYNC     = (1<<0),
    TDM_OUTPUT_MODE_FLAG_NHSYNC     = (1<<1),
    TDM_OUTPUT_MODE_FLAG_PVSYNC     = (1<<2),
    TDM_OUTPUT_MODE_FLAG_NVSYNC     = (1<<3),
    TDM_OUTPUT_MODE_FLAG_INTERLACE  = (1<<4),
    TDM_OUTPUT_MODE_FLAG_DBLSCAN    = (1<<5),
    TDM_OUTPUT_MODE_FLAG_CSYNC      = (1<<6),
    TDM_OUTPUT_MODE_FLAG_PCSYNC     = (1<<7),
    TDM_OUTPUT_MODE_FLAG_NCSYNC     = (1<<8),
    TDM_OUTPUT_MODE_FLAG_HSKEW      = (1<<9), /* hskew provided */
    TDM_OUTPUT_MODE_FLAG_BCAST      = (1<<10),
    TDM_OUTPUT_MODE_FLAG_PIXMUX     = (1<<11),
    TDM_OUTPUT_MODE_FLAG_DBLCLK     = (1<<12),
    TDM_OUTPUT_MODE_FLAG_CLKDIV2    = (1<<13),
} tdm_output_mode_flag;

typedef struct _tdm_output_mode
{
    unsigned int width;
    unsigned int height;
    unsigned int refresh;
    unsigned int flags;
    unsigned int type;
    char name[TDM_NAME_LEN];
} tdm_output_mode;

typedef struct _tdm_prop
{
    unsigned int id;
    char name[TDM_NAME_LEN];
} tdm_prop;

typedef struct _tdm_size {
    unsigned int h;
    unsigned int v;
} tdm_size;

typedef struct _tdm_pos
{
    unsigned int x;
    unsigned int y;
    unsigned int w;
    unsigned int h;
} tdm_pos;

typedef union
{
    void	 *ptr;
    int32_t  s32;
    uint32_t u32;
    int64_t  s64;
    uint64_t u64;
} tdm_value;

typedef struct _tdm_info_config
{
    tdm_size size;
    tdm_pos pos;
    tbm_format format;
} tdm_info_config;

typedef struct _tdm_info_layer
{
    tdm_info_config src_config;
    tdm_pos dst_pos;
    tdm_transform transform;
} tdm_info_layer;

typedef struct _tdm_info_pp
{
    tdm_info_config src_config;
    tdm_info_config dst_config;
    tdm_transform transform;
    int sync;
} tdm_info_pp;

typedef struct _tdm_info_capture
{
    tdm_info_config dst_config;
    tdm_transform transform;
    int oneshot;
    int frequency;
} tdm_info_capture;

typedef void tdm_display;
typedef void tdm_output;
typedef void tdm_layer;
typedef void tdm_buffer;
typedef void tdm_capture;
typedef void tdm_pp;

typedef void (*tdm_output_vblank_handler)(tdm_output *output, unsigned int sequence,
                                          unsigned int tv_sec, unsigned int tv_usec, void *user_data);
typedef void (*tdm_output_commit_handler)(tdm_output *output, unsigned int sequence,
                                          unsigned int tv_sec, unsigned int tv_usec, void *user_data);


#ifdef __cplusplus
}
#endif

#endif /* _TDM_TYPES_H_ */
