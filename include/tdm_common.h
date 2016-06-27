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

#ifndef _TDM_COMMON_H_
#define _TDM_COMMON_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TDM_NAME_LEN        64

/**
 * @file tdm_common.h
 * @brief The header file which defines Enumerations and Structures for TDM
 * frontend, backend and client.
 */

/**
 * @brief The error enumeration
 */
typedef enum {
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
	TDM_ERROR_DPMS_OFF              = -10, /**< dpms off */
} tdm_error;

/**
 * @brief The output change enumeration of #tdm_output_change_handler
 */
typedef enum {
	TDM_OUTPUT_CHANGE_CONNECTION    = (1 << 0), /**< connection chagne */
	TDM_OUTPUT_CHANGE_DPMS          = (1 << 1), /**< dpms change */
} tdm_output_change_type;

/**
 * @brief The output connection status enumeration
 */
typedef enum {
	TDM_OUTPUT_CONN_STATUS_DISCONNECTED, /**< output disconnected */
	TDM_OUTPUT_CONN_STATUS_CONNECTED,    /**< output connected */
	TDM_OUTPUT_CONN_STATUS_MODE_SETTED,  /**< output connected and setted a mode */
} tdm_output_conn_status;

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

#ifdef __cplusplus
}
#endif

#endif /* _TDM_COMMON_H_ */
