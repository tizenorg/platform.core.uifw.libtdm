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

#ifndef _TDM_LOG_H_
#define _TDM_LOG_H_


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file tdm_log.h
 * @brief The header file to print logs in frontend and backend modules
 * @details
 * The TDM debug log can be enable by setting "TDM_DEBUG" enviroment
 * @par Example
 * @code
   $ export TDM_DEBUG=1
 * @endcode
 */
extern int tdm_debug;
extern int tdm_debug_buffer;

//#define TDM_CONFIG_DLOG
#ifdef TDM_CONFIG_DLOG

#include <dlog.h>
#include <time.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "TDM"

#define TDM_DBG(fmt, args...) \
    if (tdm_debug) \
	do { \
		struct timespec ts;	\
		clock_gettime(CLOCK_MONOTONIC, &ts);	\
		LOGD("[%d.%06d] "fmt"\n", (int)ts.tv_sec, (int)ts.tv_nsec / 1000, ##args);	\
		printf("[TDM_DBG][%d.%06d][%s %d] "fmt"\n", (int)ts.tv_sec,	\
			(int)ts.tv_nsec / 1000, __func__, __LINE__, ##args); \
	} while (0);

#define TDM_INFO(fmt, args...) \
	do { \
		struct timespec ts;	\
		clock_gettime(CLOCK_MONOTONIC, &ts);	\
		LOGI("[%d.%06d] "fmt"\n", (int)ts.tv_sec, (int)ts.tv_nsec / 1000, ##args);	\
		printf("[TDM_INF][%d.%06d][%s %d] "fmt"\n", (int)ts.tv_sec,	\
			(int)ts.tv_nsec / 1000, __func__, __LINE__, ##args); \
	} while (0);

#define TDM_WRN(fmt, args...) \
	do { \
		struct timespec ts;	\
		clock_gettime(CLOCK_MONOTONIC, &ts);	\
		LOGI("[%d.%06d] "fmt"\n", (int)ts.tv_sec, (int)ts.tv_nsec / 1000, ##args);	\
		printf("[TDM_WRN][%d.%06d][%s %d] "fmt"\n", (int)ts.tv_sec,	\
			(int)ts.tv_nsec / 1000, __func__, __LINE__, ##args); \
	} while (0);

#define TDM_ERR(fmt, args...) \
	do { \
		struct timespec ts;	\
		clock_gettime(CLOCK_MONOTONIC, &ts);	\
		LOGE("[%d.%06d] "fmt"\n", (int)ts.tv_sec, (int)ts.tv_nsec / 1000, ##args);	\
		printf("[TDM_ERR][%d.%06d][%s %d] "fmt"\n", (int)ts.tv_sec,	\
			(int)ts.tv_nsec / 1000, __func__, __LINE__, ##args); \
	} while (0);

#else /* TDM_CONFIG_DLOG */

#include <stdio.h>
#include <time.h>

#define TDM_DBG(fmt, args...) \
    if (tdm_debug) \
	do { \
		struct timespec ts;	\
		clock_gettime(CLOCK_MONOTONIC, &ts);	\
		printf("[TDM_DBG][%d.%06d][%s %d] "fmt"\n", (int)ts.tv_sec,	\
			(int)ts.tv_nsec / 1000, __func__, __LINE__, ##args); \
	} while (0);

#define TDM_INFO(fmt, args...) \
	do { \
		struct timespec ts;	\
		clock_gettime(CLOCK_MONOTONIC, &ts);	\
		printf("[TDM_INF][%d.%06d][%s %d] "fmt"\n", (int)ts.tv_sec,	\
			(int)ts.tv_nsec / 1000, __func__, __LINE__, ##args); \
	} while (0);

#define TDM_WRN(fmt, args...) \
	do { \
		struct timespec ts;	\
		clock_gettime(CLOCK_MONOTONIC, &ts);	\
		printf("[TDM_WRN][%d.%06d][%s %d] "fmt"\n", (int)ts.tv_sec,	\
			(int)ts.tv_nsec / 1000, __func__, __LINE__, ##args); \
	} while (0);

#define TDM_ERR(fmt, args...) \
	do { \
		struct timespec ts;	\
		clock_gettime(CLOCK_MONOTONIC, &ts);	\
		printf("[TDM_ERR][%d.%06d][%s %d] "fmt"\n", (int)ts.tv_sec,	\
			(int)ts.tv_nsec / 1000, __func__, __LINE__, ##args); \
	} while (0);

#endif /* TDM_CONFIG_DLOG */

#ifdef __cplusplus
}
#endif

#endif /* _TDM_LOG_H_ */
