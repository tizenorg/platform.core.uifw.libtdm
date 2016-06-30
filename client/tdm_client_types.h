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

#ifndef _TDM_CLIENT_TYPES_H_
#define _TDM_CLIENT_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file tdm_client_types.h
 * @brief The header file which defines Enumerations and Structures for client.
 */

#include <tdm_common.h>

/**
 * @deprecated
 */
typedef tdm_error tdm_client_error;

/**
 * @brief The client error enumeration
 * @deprecated
 */
enum {
	TDM_CLIENT_ERROR_NONE                  = 0,  /**< none */
	TDM_CLIENT_ERROR_OPERATION_FAILED      = -1, /**< operaion failed */
	TDM_CLIENT_ERROR_INVALID_PARAMETER     = -2, /**< wrong input parameter */
	TDM_CLIENT_ERROR_PERMISSION_DENIED     = -3, /**< access denied */
	TDM_CLIENT_ERROR_OUT_OF_MEMORY         = -4, /**< no free memory */
	TDM_CLIENT_ERROR_DPMS_OFF              = -5, /**< dpms off */
};

/**
 * @deprecated
 */
typedef void
(*tdm_client_vblank_handler2)(unsigned int sequence, unsigned int tv_sec,
							  unsigned int tv_usec, void *user_data);

/**
 * @brief The TDM client object
 */
typedef void tdm_client;

/**
 * @brief The TDM client output object
 */
typedef void tdm_client_output;

/**
 * @brief The TDM client vblank object
 */
typedef void tdm_client_vblank;

/**
 * @brief The client output handler
 * @see #tdm_client_output_add_change_handler, #tdm_client_output_remove_change_handler
 */
typedef void
(*tdm_client_output_change_handler)(tdm_client_output *output,
									tdm_output_change_type type,
									tdm_value value,
									void *user_data);

/**
 * @brief The client vblank handler
 * @see #tdm_client_vblank_wait
 */
typedef void
(*tdm_client_vblank_handler)(tdm_client_vblank *vblank,
							 tdm_error error,
							 unsigned int sequence,
							 unsigned int tv_sec,
							 unsigned int tv_usec,
							 void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* _TDM_CLIENT_TYPES_H_ */
