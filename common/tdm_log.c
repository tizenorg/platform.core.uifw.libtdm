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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <sys/syscall.h>
#include <dlog.h>

#include "tdm.h"
#include "tdm_log.h"
#include "tdm_macro.h"

//#define TDM_CONFIG_ASSERT

#define LOG_MAX_LEN    4076

#define COLOR_RED "\x1b[31m"      /* for error */
#define COLOR_YELLOW "\x1b[33m"   /* for warning */
#define COLOR_GREEN "\x1b[32m"    /* for info */
#define COLOR_RESET "\x1b[0m"

#undef LOG_TAG
#define LOG_TAG "TDM"

static unsigned int dlog_enable;
static unsigned int debug_enable;

static unsigned int need_check_env = 1;

static void
_tdm_log_check_env(void)
{
	const char *str;

	str = getenv("TDM_DEBUG");
	if (str && (strstr(str, "1")))
		debug_enable = 1;

	str = getenv("TDM_DLOG");
	if (str && (strstr(str, "1")))
		dlog_enable = 1;
}

EXTERN void
tdm_log_enable_dlog(unsigned int enable)
{
	dlog_enable = enable;
}

EXTERN void
tdm_log_enable_debug(unsigned int enable)
{
	debug_enable = enable;
}

EXTERN void
tdm_log_print(int level, const char *fmt, ...)
{
	va_list arg;

	if (need_check_env) {
		need_check_env = 0;
		_tdm_log_check_env();
	}

	if (level > 3 && !debug_enable)
		return;

	if (dlog_enable) {
		log_priority dlog_prio;
		switch (level) {
		case TDM_LOG_LEVEL_ERR:
			dlog_prio = DLOG_ERROR;
			break;
		case TDM_LOG_LEVEL_WRN:
			dlog_prio = DLOG_WARN;
			break;
		case TDM_LOG_LEVEL_INFO:
			dlog_prio = DLOG_INFO;
			break;
		case TDM_LOG_LEVEL_DBG:
			dlog_prio = DLOG_DEBUG;
			break;
		default:
			return;
		}
		va_start(arg, fmt);
		dlog_vprint(dlog_prio, LOG_TAG, fmt, arg);
		va_end(arg);
	} else {
		struct timespec ts;
		char *lvl_str[] = {"TDM_NON", "TDM_ERR", "TDM_WRN", "TDM_INF", "TDM_DBG"};
		char *color[] = {COLOR_RESET, COLOR_RED, COLOR_YELLOW, COLOR_GREEN, COLOR_RESET};

		clock_gettime(CLOCK_MONOTONIC, &ts);

		printf("%s", color[level]);
		printf("[%s]", lvl_str[level]);
		printf(COLOR_RESET"[%d.%06d]", (int)ts.tv_sec, (int)ts.tv_nsec / 1000);
		va_start(arg, fmt);
		vprintf(fmt, arg);
		va_end(arg);
	}

#ifdef TDM_CONFIG_ASSERT
	if (level < 3)
		assert(0);
#endif
}
