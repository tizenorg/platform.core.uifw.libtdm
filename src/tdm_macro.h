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
#include <tbm_surface.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TDM_SERVER_REPLY_MSG_LEN	8192

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
#define TDM_EXIT_IF_FAIL(cond) { \
	if (!(cond)) { \
		TDM_ERR("'%s' failed", #cond); \
		exit(0); \
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

#define TDM_DBG_RETURN_IF_FAIL(cond) { \
	if (!(cond))  { \
		TDM_SNPRINTF(reply, len, "[%s %d] '%s' failed\n", __func__, __LINE__, #cond); \
		return; \
	} \
}
#define TDM_DBG_GOTO_IF_FAIL(cond, dst) { \
	if (!(cond))  { \
		TDM_SNPRINTF(reply, len, "[%s %d] '%s' failed\n", __func__, __LINE__, #cond); \
		goto dst; \
	} \
}

#define C(b, m)             (((b) >> (m)) & 0xFF)
#define B(c, s)             ((((unsigned int)(c)) & 0xff) << (s))
#define FOURCC(a, b, c, d)  (B(d, 24) | B(c, 16) | B(b, 8) | B(a, 0))
#define FOURCC_STR(id)      C(id, 0), C(id, 8), C(id, 16), C(id, 24)
#define FOURCC_ID(str)      FOURCC(((char*)str)[0], ((char*)str)[1], ((char*)str)[2], ((char*)str)[3])
#define IS_RGB(f)           ((f) == TBM_FORMAT_XRGB8888 || (f) == TBM_FORMAT_ARGB8888)

/* don't using !,$,# */
#define TDM_DELIM           "@^&*+-|,:~"
#define TDM_ALIGN(a, b)     (((a) + ((b) - 1)) & ~((b) - 1))

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

static struct tdm_type_name tdm_conn_names[] = {
	{ TDM_OUTPUT_TYPE_Unknown, "Unknown" },
	{ TDM_OUTPUT_TYPE_VGA, "VGA" },
	{ TDM_OUTPUT_TYPE_DVII, "DVII" },
	{ TDM_OUTPUT_TYPE_DVID, "DVID" },
	{ TDM_OUTPUT_TYPE_DVIA, "DVIA" },
	{ TDM_OUTPUT_TYPE_Composite, "Composite" },
	{ TDM_OUTPUT_TYPE_SVIDEO, "SVIDEO" },
	{ TDM_OUTPUT_TYPE_LVDS, "LVDS" },
	{ TDM_OUTPUT_TYPE_Component, "Component" },
	{ TDM_OUTPUT_TYPE_9PinDIN, "9PinDIN" },
	{ TDM_OUTPUT_TYPE_DisplayPort, "DisplayPort" },
	{ TDM_OUTPUT_TYPE_HDMIA, "HDMIA" },
	{ TDM_OUTPUT_TYPE_HDMIB, "HDMIB" },
	{ TDM_OUTPUT_TYPE_TV, "TV" },
	{ TDM_OUTPUT_TYPE_eDP, "eDP" },
	{ TDM_OUTPUT_TYPE_VIRTUAL, "VIRTUAL" },
	{ TDM_OUTPUT_TYPE_DSI, "DSI" },
};
TDM_TYPE_NAME_FN(conn)


#define TDM_BIT_NAME_FB(res)					\
static inline const char * tdm_##res##_str(int type, char **reply, int *len)	\
{			\
	unsigned int i;						\
	const char *sep = "";					\
	for (i = 0; i < TDM_ARRAY_SIZE(tdm_##res##_names); i++) {		\
		if (type & (1 << i)) {				\
			TDM_SNPRINTF(*reply, len, "%s%s", sep, tdm_##res##_names[i]);	\
			sep = ",";				\
		}						\
	}							\
	return NULL;						\
}

static const char *tdm_mode_type_names[] = {
	"builtin",
	"clock_c",
	"crtc_c",
	"preferred",
	"default",
	"userdef",
	"driver",
};
TDM_BIT_NAME_FB(mode_type)

static const char *tdm_mode_flag_names[] = {
	"phsync",
	"nhsync",
	"pvsync",
	"nvsync",
	"interlace",
	"dblscan",
	"csync",
	"pcsync",
	"ncsync",
	"hskew",
	"bcast",
	"pixmux",
	"dblclk",
	"clkdiv2"
};
TDM_BIT_NAME_FB(mode_flag)

static const char *tdm_layer_caps_names[] = {
	"cursor",
	"primary",
	"overlay",
	"",
	"graphic",
	"video",
	"",
	"",
	"scale",
	"transform",
	"scanout",
	"reserved",
	"no_crop",
};
TDM_BIT_NAME_FB(layer_caps)

static const char *tdm_pp_caps_names[] = {
	"sync",
	"async",
	"scale",
	"transform",
};
TDM_BIT_NAME_FB(pp_caps)

static const char *tdm_capture_caps_names[] = {
	"output",
	"layer",
	"scale",
	"transform",
};
TDM_BIT_NAME_FB(capture_caps)

static inline char*
strtostr(char *buf, int len, char *str, char *delim)
{
	char *end;
	end = strpbrk(str, delim);
	if (end)
		len = ((end - str + 1) < len) ? (end - str + 1) : len;
	else {
		int l = strlen(str);
		len = ((l + 1) < len) ? (l + 1) : len;
	}
	snprintf(buf, len, "%s", str);
	return str + len - 1;
}

#ifdef __cplusplus
}
#endif

#endif /* _TDM_MACRO_H_ */
