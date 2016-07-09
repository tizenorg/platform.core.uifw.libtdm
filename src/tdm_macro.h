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

#ifndef _TDM_MACRO_H_
#define _TDM_MACRO_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include <tdm_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#undef EXTERN
#undef DEPRECATED
#undef INTERN

#if defined(__GNUC__) && __GNUC__ >= 4
#define EXTERN __attribute__ ((visibility("default")))
#else
#define EXTERN
#endif

#if defined(__GNUC__) && __GNUC__ >= 4
#define INTERN __attribute__ ((visibility("hidden")))
#else
#define INTERN
#endif

#if defined(__GNUC__) && __GNUC__ >= 4
#define DEPRECATED __attribute__ ((deprecated))
#else
#define DEPRECATED
#endif

/* check condition */
#define TDM_RETURN_IF_FAIL(cond) { \
	if (!(cond))  { \
		TDM_ERR("'%s' failed", #cond); \
		return; \
	} \
}
#define TDM_RETURN_VAL_IF_FAIL(cond, val) { \
	if (!(cond)) { \
		TDM_ERR("'%s' failed", #cond); \
		return val; \
	} \
}
#define TDM_RETURN_VAL_IF_FAIL_WITH_ERROR(cond, error_v, val) { \
	if (!(cond)) { \
		TDM_ERR("'%s' failed", #cond); \
		ret = error_v; \
		if (error) *error = ret; \
		return val; \
	} \
}

#define TDM_WARNING_IF_FAIL(cond)  { \
	if (!(cond)) \
		TDM_ERR("'%s' failed", #cond); \
}
#define TDM_GOTO_IF_FAIL(cond, dst) { \
	if (!(cond)) { \
		TDM_ERR("'%s' failed", #cond); \
		goto dst; \
	} \
}

#define TDM_NEVER_GET_HERE() TDM_WRN("** NEVER GET HERE **")

#define TDM_SNPRINTF(p, len, fmt, ARG...)  \
	do { \
		if (p && len && *len > 0) { \
			int s = snprintf(p, *len, fmt, ##ARG); \
			p += s; \
			*len -= s; \
		} \
	} while (0)

#define C(b, m)             (((b) >> (m)) & 0xFF)
#define B(c, s)             ((((unsigned int)(c)) & 0xff) << (s))
#define FOURCC(a, b, c, d)  (B(d, 24) | B(c, 16) | B(b, 8) | B(a, 0))
#define FOURCC_STR(id)      C(id, 0), C(id, 8), C(id, 16), C(id, 24)
#define FOURCC_ID(str)      FOURCC(((char*)str)[0], ((char*)str)[1], ((char*)str)[2], ((char*)str)[3])


#define TDM_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct tdm_type_name {
	int type;
	const char *name;
};

#define TDM_TYPE_NAME_FN(res) \
static inline const char * tdm_##res##_str(int type)	\
{			\
	unsigned int i;					\
	for (i = 0; i < TDM_ARRAY_SIZE(tdm_##res##_names); i++) { \
		if (tdm_##res##_names[i].type == type)	\
			return tdm_##res##_names[i].name;	\
	}						\
	return "(invalid)";				\
}

static struct tdm_type_name tdm_dpms_names[] = {
	{ TDM_OUTPUT_DPMS_ON, "on" },
	{ TDM_OUTPUT_DPMS_STANDBY, "standby" },
	{ TDM_OUTPUT_DPMS_SUSPEND, "suspend" },
	{ TDM_OUTPUT_DPMS_OFF, "off" },
};

TDM_TYPE_NAME_FN(dpms)

static struct tdm_type_name tdm_status_names[] = {
	{ TDM_OUTPUT_CONN_STATUS_DISCONNECTED, "disconnected" },
	{ TDM_OUTPUT_CONN_STATUS_CONNECTED, "connected" },
	{ TDM_OUTPUT_CONN_STATUS_MODE_SETTED, "mode_setted" },
};

TDM_TYPE_NAME_FN(status)

#ifdef __cplusplus
}
#endif

#endif /* _TDM_MACRO_H_ */
