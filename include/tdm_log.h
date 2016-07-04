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

#ifndef _TDM_LOG_H_
#define _TDM_LOG_H_

#include <unistd.h>
#include <sys/syscall.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file tdm_log.h
 * @brief The header file to print logs in frontend and backend modules
 * @details
 * The TDM debug log can be enable by setting "TDM_DEBUG" enviroment. And also,
 * the TDM dlog can be enable by setting "TDM_DLOG" enviroment.
 * @par Example
 * @code
 *  $ export TDM_DEBUG=1
 * @endcode
 */

enum {
	TDM_LOG_LEVEL_NONE,
	TDM_LOG_LEVEL_ERR,
	TDM_LOG_LEVEL_WRN,
	TDM_LOG_LEVEL_INFO,
	TDM_LOG_LEVEL_DBG,
};

void tdm_log_enable_dlog(unsigned int enable);
void tdm_log_enable_debug(unsigned int enable);
void tdm_log_set_debug_level(int level);
void tdm_log_print(int level, const char *fmt, ...);

#define TDM_DBG(fmt, args...) \
	tdm_log_print(TDM_LOG_LEVEL_DBG, "[%d][%s %d]"fmt"\n", \
				  (int)syscall(SYS_gettid), __FUNCTION__, __LINE__, ##args)
#define TDM_INFO(fmt, args...) \
	tdm_log_print(TDM_LOG_LEVEL_INFO, "[%d][%s %d]"fmt"\n", \
				  (int)syscall(SYS_gettid), __FUNCTION__, __LINE__, ##args)
#define TDM_WRN(fmt, args...) \
	tdm_log_print(TDM_LOG_LEVEL_WRN, "[%d][%s %d]"fmt"\n", \
				  (int)syscall(SYS_gettid), __FUNCTION__, __LINE__, ##args)
#define TDM_ERR(fmt, args...) \
	tdm_log_print(TDM_LOG_LEVEL_ERR, "[%d][%s %d]"fmt"\n", \
				  (int)syscall(SYS_gettid), __FUNCTION__, __LINE__, ##args)

#ifdef __cplusplus
}
#endif

#endif /* _TDM_LOG_H_ */
