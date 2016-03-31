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

#ifndef _TDM_CLIENT_H_
#define _TDM_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file tdm_client.h
 * @brief The header file for a client user of TDM.
 * @par Example
 * @code
   #include <tdm_client.h>    //for a client user of TDM
 * @endcode
 */

typedef enum
{
	TDM_CLIENT_ERROR_NONE                  = 0,  /**< none */
	TDM_CLIENT_ERROR_OPERATION_FAILED      = -1, /**< operaion failed */
	TDM_CLIENT_ERROR_INVALID_PARAMETER     = -2, /**< wrong input parameter */
	TDM_CLIENT_ERROR_PERMISSION_DENIED     = -3, /**< access denied */
	TDM_CLIENT_ERROR_OUT_OF_MEMORY         = -4, /**< no free memory */
} tdm_client_error;

typedef void *tdm_client;

typedef void
(*tdm_client_vblank_handler)(unsigned int sequence, unsigned int tv_sec,
                             unsigned int tv_usec, void *user_data);

tdm_client*
tdm_client_create(tdm_client_error *error);

void
tdm_client_destroy(tdm_client *client);

tdm_client_error
tdm_client_get_fd(tdm_client *client, int *fd);

tdm_client_error
tdm_client_handle_events(tdm_client *client);

tdm_client_error
tdm_client_wait_vblank(tdm_client *client, char *name, int interval, int sync,
                       tdm_client_vblank_handler func, void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* _TDM_CLIENT_H_ */
