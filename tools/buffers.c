/*
 * DRM based mode setting test program
 * Copyright 2008 Tungsten Graphics
 *   Jakob Bornecrantz <jakob@tungstengraphics.com>
 * Copyright 2008 Intel Corporation
 *   Jesse Barnes <jesse.barnes@intel.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <tbm_bufmgr.h>
#include <tbm_surface.h>

#include <tdm_log.h>
#include "tdm_macro.h"
#include "buffers.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* -----------------------------------------------------------------------------
 * Formats
 */

struct color_component {
	unsigned int length;
	unsigned int offset;
};

struct rgb_info {
	struct color_component red;
	struct color_component green;
	struct color_component blue;
	struct color_component alpha;
};

enum yuv_order {
	YUV_YCbCr = 1,
	YUV_YCrCb = 2,
	YUV_YC = 4,
	YUV_CY = 8,
};

struct yuv_info {
	enum yuv_order order;
	unsigned int xsub;
	unsigned int ysub;
	unsigned int chroma_stride;
};

struct format_info {
	unsigned int format;
	const char *name;
	const struct rgb_info rgb;
	const struct yuv_info yuv;
};

#define MAKE_RGB_INFO(rl, ro, bl, bo, gl, go, al, ao) \
	.rgb = { { (rl), (ro) }, { (bl), (bo) }, { (gl), (go) }, { (al), (ao) } }

#define MAKE_YUV_INFO(order, xsub, ysub, chroma_stride) \
	.yuv = { (order), (xsub), (ysub), (chroma_stride) }

static const struct format_info format_info[] = {
	/* YUV packed */
	{ TBM_FORMAT_UYVY, "UYVY", MAKE_YUV_INFO(YUV_YCbCr | YUV_CY, 2, 2, 2) },
	{ TBM_FORMAT_VYUY, "VYUY", MAKE_YUV_INFO(YUV_YCrCb | YUV_CY, 2, 2, 2) },
	{ TBM_FORMAT_YUYV, "YUYV", MAKE_YUV_INFO(YUV_YCbCr | YUV_YC, 2, 2, 2) },
	{ TBM_FORMAT_YVYU, "YVYU", MAKE_YUV_INFO(YUV_YCrCb | YUV_YC, 2, 2, 2) },
	/* YUV semi-planar */
	{ TBM_FORMAT_NV12, "NV12", MAKE_YUV_INFO(YUV_YCbCr, 2, 2, 2) },
	{ TBM_FORMAT_NV21, "NV21", MAKE_YUV_INFO(YUV_YCrCb, 2, 2, 2) },
	{ TBM_FORMAT_NV16, "NV16", MAKE_YUV_INFO(YUV_YCbCr, 2, 1, 2) },
	{ TBM_FORMAT_NV61, "NV61", MAKE_YUV_INFO(YUV_YCrCb, 2, 1, 2) },
	/* YUV planar */
	{ TBM_FORMAT_YUV420, "YU12", MAKE_YUV_INFO(YUV_YCbCr, 2, 2, 1) },
	{ TBM_FORMAT_YVU420, "YV12", MAKE_YUV_INFO(YUV_YCrCb, 2, 2, 1) },
	/* RGB16 */
	{ TBM_FORMAT_ARGB4444, "AR12", MAKE_RGB_INFO(4, 8, 4, 4, 4, 0, 4, 12) },
	{ TBM_FORMAT_XRGB4444, "XR12", MAKE_RGB_INFO(4, 8, 4, 4, 4, 0, 0, 0) },
	{ TBM_FORMAT_ABGR4444, "AB12", MAKE_RGB_INFO(4, 0, 4, 4, 4, 8, 4, 12) },
	{ TBM_FORMAT_XBGR4444, "XB12", MAKE_RGB_INFO(4, 0, 4, 4, 4, 8, 0, 0) },
	{ TBM_FORMAT_RGBA4444, "RA12", MAKE_RGB_INFO(4, 12, 4, 8, 4, 4, 4, 0) },
	{ TBM_FORMAT_RGBX4444, "RX12", MAKE_RGB_INFO(4, 12, 4, 8, 4, 4, 0, 0) },
	{ TBM_FORMAT_BGRA4444, "BA12", MAKE_RGB_INFO(4, 4, 4, 8, 4, 12, 4, 0) },
	{ TBM_FORMAT_BGRX4444, "BX12", MAKE_RGB_INFO(4, 4, 4, 8, 4, 12, 0, 0) },
	{ TBM_FORMAT_ARGB1555, "AR15", MAKE_RGB_INFO(5, 10, 5, 5, 5, 0, 1, 15) },
	{ TBM_FORMAT_XRGB1555, "XR15", MAKE_RGB_INFO(5, 10, 5, 5, 5, 0, 0, 0) },
	{ TBM_FORMAT_ABGR1555, "AB15", MAKE_RGB_INFO(5, 0, 5, 5, 5, 10, 1, 15) },
	{ TBM_FORMAT_XBGR1555, "XB15", MAKE_RGB_INFO(5, 0, 5, 5, 5, 10, 0, 0) },
	{ TBM_FORMAT_RGBA5551, "RA15", MAKE_RGB_INFO(5, 11, 5, 6, 5, 1, 1, 0) },
	{ TBM_FORMAT_RGBX5551, "RX15", MAKE_RGB_INFO(5, 11, 5, 6, 5, 1, 0, 0) },
	{ TBM_FORMAT_BGRA5551, "BA15", MAKE_RGB_INFO(5, 1, 5, 6, 5, 11, 1, 0) },
	{ TBM_FORMAT_BGRX5551, "BX15", MAKE_RGB_INFO(5, 1, 5, 6, 5, 11, 0, 0) },
	{ TBM_FORMAT_RGB565, "RG16", MAKE_RGB_INFO(5, 11, 6, 5, 5, 0, 0, 0) },
	{ TBM_FORMAT_BGR565, "BG16", MAKE_RGB_INFO(5, 0, 6, 5, 5, 11, 0, 0) },
	/* RGB24 */
	{ TBM_FORMAT_BGR888, "BG24", MAKE_RGB_INFO(8, 0, 8, 8, 8, 16, 0, 0) },
	{ TBM_FORMAT_RGB888, "RG24", MAKE_RGB_INFO(8, 16, 8, 8, 8, 0, 0, 0) },
	/* RGB32 */
	{ TBM_FORMAT_ARGB8888, "AR24", MAKE_RGB_INFO(8, 16, 8, 8, 8, 0, 8, 24) },
	{ TBM_FORMAT_XRGB8888, "XR24", MAKE_RGB_INFO(8, 16, 8, 8, 8, 0, 0, 0) },
	{ TBM_FORMAT_ABGR8888, "AB24", MAKE_RGB_INFO(8, 0, 8, 8, 8, 16, 8, 24) },
	{ TBM_FORMAT_XBGR8888, "XB24", MAKE_RGB_INFO(8, 0, 8, 8, 8, 16, 0, 0) },
	{ TBM_FORMAT_RGBA8888, "RA24", MAKE_RGB_INFO(8, 24, 8, 16, 8, 8, 8, 0) },
	{ TBM_FORMAT_RGBX8888, "RX24", MAKE_RGB_INFO(8, 24, 8, 16, 8, 8, 0, 0) },
	{ TBM_FORMAT_BGRA8888, "BA24", MAKE_RGB_INFO(8, 8, 8, 16, 8, 24, 8, 0) },
	{ TBM_FORMAT_BGRX8888, "BX24", MAKE_RGB_INFO(8, 8, 8, 16, 8, 24, 0, 0) },
	{ TBM_FORMAT_ARGB2101010, "AR30", MAKE_RGB_INFO(10, 20, 10, 10, 10, 0, 2, 30) },
	{ TBM_FORMAT_XRGB2101010, "XR30", MAKE_RGB_INFO(10, 20, 10, 10, 10, 0, 0, 0) },
	{ TBM_FORMAT_ABGR2101010, "AB30", MAKE_RGB_INFO(10, 0, 10, 10, 10, 20, 2, 30) },
	{ TBM_FORMAT_XBGR2101010, "XB30", MAKE_RGB_INFO(10, 0, 10, 10, 10, 20, 0, 0) },
	{ TBM_FORMAT_RGBA1010102, "RA30", MAKE_RGB_INFO(10, 22, 10, 12, 10, 2, 2, 0) },
	{ TBM_FORMAT_RGBX1010102, "RX30", MAKE_RGB_INFO(10, 22, 10, 12, 10, 2, 0, 0) },
	{ TBM_FORMAT_BGRA1010102, "BA30", MAKE_RGB_INFO(10, 2, 10, 12, 10, 22, 2, 0) },
	{ TBM_FORMAT_BGRX1010102, "BX30", MAKE_RGB_INFO(10, 2, 10, 12, 10, 22, 0, 0) },
};

static unsigned int rand_seed;

unsigned int format_fourcc(const char *name)
{
	unsigned int i;
	for (i = 0; i < ARRAY_SIZE(format_info); i++) {
		if (!strcmp(format_info[i].name, name))
			return format_info[i].format;
	}
	return 0;
}

/* -----------------------------------------------------------------------------
 * Test patterns
 */

struct color_rgb24 {
	unsigned int value: 24;
} __attribute__((__packed__));

struct color_yuv {
	unsigned char y;
	unsigned char u;
	unsigned char v;
};

#define MAKE_YUV_601_Y(r, g, b) \
	((( 66 * (r) + 129 * (g) +  25 * (b) + 128) >> 8) + 16)
#define MAKE_YUV_601_U(r, g, b) \
	(((-38 * (r) -  74 * (g) + 112 * (b) + 128) >> 8) + 128)
#define MAKE_YUV_601_V(r, g, b) \
	(((112 * (r) -  94 * (g) -  18 * (b) + 128) >> 8) + 128)

#define MAKE_YUV_601(r, g, b) \
	{ .y = MAKE_YUV_601_Y(r, g, b), \
	  .u = MAKE_YUV_601_U(r, g, b), \
	  .v = MAKE_YUV_601_V(r, g, b) }

#define MAKE_RGBA(rgb, r, g, b, a) \
	((((r) >> (8 - (rgb)->red.length)) << (rgb)->red.offset) | \
	 (((g) >> (8 - (rgb)->green.length)) << (rgb)->green.offset) | \
	 (((b) >> (8 - (rgb)->blue.length)) << (rgb)->blue.offset) | \
	 (((a) >> (8 - (rgb)->alpha.length)) << (rgb)->alpha.offset))

#define MAKE_RGB24(rgb, r, g, b) \
	{ .value = MAKE_RGBA(rgb, r, g, b, 0) }

static void
fill_smpte_yuv_planar(const struct yuv_info *yuv,
					  unsigned char *y_mem, unsigned char *u_mem,
					  unsigned char *v_mem, unsigned int width,
					  unsigned int height, unsigned int stride)
{
	const struct color_yuv colors_top[] = {
		MAKE_YUV_601(191, 192, 192),	/* grey */
		MAKE_YUV_601(192, 192, 0),	/* yellow */
		MAKE_YUV_601(0, 192, 192),	/* cyan */
		MAKE_YUV_601(0, 192, 0),	/* green */
		MAKE_YUV_601(192, 0, 192),	/* magenta */
		MAKE_YUV_601(192, 0, 0),	/* red */
		MAKE_YUV_601(0, 0, 192),	/* blue */
	};
	const struct color_yuv colors_middle[] = {
		MAKE_YUV_601(0, 0, 192),	/* blue */
		MAKE_YUV_601(19, 19, 19),	/* black */
		MAKE_YUV_601(192, 0, 192),	/* magenta */
		MAKE_YUV_601(19, 19, 19),	/* black */
		MAKE_YUV_601(0, 192, 192),	/* cyan */
		MAKE_YUV_601(19, 19, 19),	/* black */
		MAKE_YUV_601(192, 192, 192),	/* grey */
	};
	const struct color_yuv colors_bottom[] = {
		MAKE_YUV_601(0, 33, 76),	/* in-phase */
		MAKE_YUV_601(255, 255, 255),	/* super white */
		MAKE_YUV_601(50, 0, 106),	/* quadrature */
		MAKE_YUV_601(19, 19, 19),	/* black */
		MAKE_YUV_601(9, 9, 9),		/* 3.5% */
		MAKE_YUV_601(19, 19, 19),	/* 7.5% */
		MAKE_YUV_601(29, 29, 29),	/* 11.5% */
		MAKE_YUV_601(19, 19, 19),	/* black */
	};
	unsigned int cs = yuv->chroma_stride;
	unsigned int xsub = yuv->xsub;
	unsigned int ysub = yuv->ysub;
	unsigned int x;
	unsigned int y;

	/* Luma */
	for (y = 0; y < height * 6 / 9; ++y) {
		for (x = 0; x < width; ++x)
			y_mem[x] = colors_top[x * 7 / width].y;
		y_mem += stride;
	}

	for (; y < height * 7 / 9; ++y) {
		for (x = 0; x < width; ++x)
			y_mem[x] = colors_middle[x * 7 / width].y;
		y_mem += stride;
	}

	for (; y < height; ++y) {
		for (x = 0; x < width * 5 / 7; ++x)
			y_mem[x] = colors_bottom[x * 4 / (width * 5 / 7)].y;
		for (; x < width * 6 / 7; ++x)
			y_mem[x] = colors_bottom[(x - width * 5 / 7) * 3
									 / (width / 7) + 4].y;
		for (; x < width; ++x)
			y_mem[x] = colors_bottom[7].y;
		y_mem += stride;
	}

	/* Chroma */
	for (y = 0; y < height / ysub * 6 / 9; ++y) {
		for (x = 0; x < width; x += xsub) {
			u_mem[x * cs / xsub] = colors_top[x * 7 / width].u;
			v_mem[x * cs / xsub] = colors_top[x * 7 / width].v;
		}
		u_mem += stride * cs / xsub;
		v_mem += stride * cs / xsub;
	}

	for (; y < height / ysub * 7 / 9; ++y) {
		for (x = 0; x < width; x += xsub) {
			u_mem[x * cs / xsub] = colors_middle[x * 7 / width].u;
			v_mem[x * cs / xsub] = colors_middle[x * 7 / width].v;
		}
		u_mem += stride * cs / xsub;
		v_mem += stride * cs / xsub;
	}

	for (; y < height / ysub; ++y) {
		for (x = 0; x < width * 5 / 7; x += xsub) {
			u_mem[x * cs / xsub] =
				colors_bottom[x * 4 / (width * 5 / 7)].u;
			v_mem[x * cs / xsub] =
				colors_bottom[x * 4 / (width * 5 / 7)].v;
		}
		for (; x < width * 6 / 7; x += xsub) {
			u_mem[x * cs / xsub] = colors_bottom[(x - width * 5 / 7) *
												 3 / (width / 7) + 4].u;
			v_mem[x * cs / xsub] = colors_bottom[(x - width * 5 / 7) *
												 3 / (width / 7) + 4].v;
		}
		for (; x < width; x += xsub) {
			u_mem[x * cs / xsub] = colors_bottom[7].u;
			v_mem[x * cs / xsub] = colors_bottom[7].v;
		}
		u_mem += stride * cs / xsub;
		v_mem += stride * cs / xsub;
	}
}

static void
fill_smpte_yuv_packed(const struct yuv_info *yuv, unsigned char *mem,
					  unsigned int width, unsigned int height,
					  unsigned int stride)
{
	const struct color_yuv colors_top[] = {
		MAKE_YUV_601(191, 192, 192),	/* grey */
		MAKE_YUV_601(192, 192, 0),	/* yellow */
		MAKE_YUV_601(0, 192, 192),	/* cyan */
		MAKE_YUV_601(0, 192, 0),	/* green */
		MAKE_YUV_601(192, 0, 192),	/* magenta */
		MAKE_YUV_601(192, 0, 0),	/* red */
		MAKE_YUV_601(0, 0, 192),	/* blue */
	};
	const struct color_yuv colors_middle[] = {
		MAKE_YUV_601(0, 0, 192),	/* blue */
		MAKE_YUV_601(19, 19, 19),	/* black */
		MAKE_YUV_601(192, 0, 192),	/* magenta */
		MAKE_YUV_601(19, 19, 19),	/* black */
		MAKE_YUV_601(0, 192, 192),	/* cyan */
		MAKE_YUV_601(19, 19, 19),	/* black */
		MAKE_YUV_601(192, 192, 192),	/* grey */
	};
	const struct color_yuv colors_bottom[] = {
		MAKE_YUV_601(0, 33, 76),	/* in-phase */
		MAKE_YUV_601(255, 255, 255),	/* super white */
		MAKE_YUV_601(50, 0, 106),	/* quadrature */
		MAKE_YUV_601(19, 19, 19),	/* black */
		MAKE_YUV_601(9, 9, 9),		/* 3.5% */
		MAKE_YUV_601(19, 19, 19),	/* 7.5% */
		MAKE_YUV_601(29, 29, 29),	/* 11.5% */
		MAKE_YUV_601(19, 19, 19),	/* black */
	};
	unsigned char *y_mem = (yuv->order & YUV_YC) ? mem : mem + 1;
	unsigned char *c_mem = (yuv->order & YUV_CY) ? mem : mem + 1;
	unsigned int u = (yuv->order & YUV_YCrCb) ? 2 : 0;
	unsigned int v = (yuv->order & YUV_YCbCr) ? 2 : 0;
	unsigned int x;
	unsigned int y;

	if (width < 8)
		return;

	/* Luma */
	for (y = 0; y < height * 6 / 9; ++y) {
		for (x = 0; x < width; ++x)
			y_mem[2 * x] = colors_top[x * 7 / width].y;
		y_mem += stride;
	}

	for (; y < height * 7 / 9; ++y) {
		for (x = 0; x < width; ++x)
			y_mem[2 * x] = colors_middle[x * 7 / width].y;
		y_mem += stride;
	}

	for (; y < height; ++y) {
		for (x = 0; x < width * 5 / 7; ++x)
			y_mem[2 * x] = colors_bottom[x * 4 / (width * 5 / 7)].y;
		for (; x < width * 6 / 7; ++x)
			y_mem[2 * x] = colors_bottom[(x - width * 5 / 7) * 3
										 / (width / 7) + 4].y;
		for (; x < width; ++x)
			y_mem[2 * x] = colors_bottom[7].y;
		y_mem += stride;
	}

	/* Chroma */
	for (y = 0; y < height * 6 / 9; ++y) {
		for (x = 0; x < width; x += 2) {
			c_mem[2 * x + u] = colors_top[x * 7 / width].u;
			c_mem[2 * x + v] = colors_top[x * 7 / width].v;
		}
		c_mem += stride;
	}

	for (; y < height * 7 / 9; ++y) {
		for (x = 0; x < width; x += 2) {
			c_mem[2 * x + u] = colors_middle[x * 7 / width].u;
			c_mem[2 * x + v] = colors_middle[x * 7 / width].v;
		}
		c_mem += stride;
	}

	for (; y < height; ++y) {
		for (x = 0; x < width * 5 / 7; x += 2) {
			c_mem[2 * x + u] = colors_bottom[x * 4 / (width * 5 / 7)].u;
			c_mem[2 * x + v] = colors_bottom[x * 4 / (width * 5 / 7)].v;
		}
		for (; x < width * 6 / 7; x += 2) {
			c_mem[2 * x + u] = colors_bottom[(x - width * 5 / 7) *
											 3 / (width / 7) + 4].u;
			c_mem[2 * x + v] = colors_bottom[(x - width * 5 / 7) *
											 3 / (width / 7) + 4].v;
		}
		for (; x < width; x += 2) {
			c_mem[2 * x + u] = colors_bottom[7].u;
			c_mem[2 * x + v] = colors_bottom[7].v;
		}
		c_mem += stride;
	}
}

static void
fill_smpte_rgb16(const struct rgb_info *rgb, unsigned char *mem,
				 unsigned int width, unsigned int height, unsigned int stride)
{
	const uint16_t colors_top[] = {
		MAKE_RGBA(rgb, 192, 192, 192, 255),	/* grey */
		MAKE_RGBA(rgb, 192, 192, 0, 255),	/* yellow */
		MAKE_RGBA(rgb, 0, 192, 192, 255),	/* cyan */
		MAKE_RGBA(rgb, 0, 192, 0, 255),		/* green */
		MAKE_RGBA(rgb, 192, 0, 192, 255),	/* magenta */
		MAKE_RGBA(rgb, 192, 0, 0, 255),		/* red */
		MAKE_RGBA(rgb, 0, 0, 192, 255),		/* blue */
	};
	const uint16_t colors_middle[] = {
		MAKE_RGBA(rgb, 0, 0, 192, 255),		/* blue */
		MAKE_RGBA(rgb, 19, 19, 19, 255),	/* black */
		MAKE_RGBA(rgb, 192, 0, 192, 255),	/* magenta */
		MAKE_RGBA(rgb, 19, 19, 19, 255),	/* black */
		MAKE_RGBA(rgb, 0, 192, 192, 255),	/* cyan */
		MAKE_RGBA(rgb, 19, 19, 19, 255),	/* black */
		MAKE_RGBA(rgb, 192, 192, 192, 255),	/* grey */
	};
	const uint16_t colors_bottom[] = {
		MAKE_RGBA(rgb, 0, 33, 76, 255),		/* in-phase */
		MAKE_RGBA(rgb, 255, 255, 255, 255),	/* super white */
		MAKE_RGBA(rgb, 50, 0, 106, 255),	/* quadrature */
		MAKE_RGBA(rgb, 19, 19, 19, 255),	/* black */
		MAKE_RGBA(rgb, 9, 9, 9, 255),		/* 3.5% */
		MAKE_RGBA(rgb, 19, 19, 19, 255),	/* 7.5% */
		MAKE_RGBA(rgb, 29, 29, 29, 255),	/* 11.5% */
		MAKE_RGBA(rgb, 19, 19, 19, 255),	/* black */
	};
	unsigned int x;
	unsigned int y;

	if (width < 8)
		return;

	for (y = 0; y < height * 6 / 9; ++y) {
		for (x = 0; x < width; ++x)
			((uint16_t *)mem)[x] = colors_top[x * 7 / width];
		mem += stride;
	}

	for (; y < height * 7 / 9; ++y) {
		for (x = 0; x < width; ++x)
			((uint16_t *)mem)[x] = colors_middle[x * 7 / width];
		mem += stride;
	}

	for (; y < height; ++y) {
		for (x = 0; x < width * 5 / 7; ++x)
			((uint16_t *)mem)[x] =
				colors_bottom[x * 4 / (width * 5 / 7)];
		for (; x < width * 6 / 7; ++x)
			((uint16_t *)mem)[x] =
				colors_bottom[(x - width * 5 / 7) * 3
							  / (width / 7) + 4];
		for (; x < width; ++x)
			((uint16_t *)mem)[x] = colors_bottom[7];
		mem += stride;
	}
}

static void
fill_smpte_rgb24(const struct rgb_info *rgb, void *mem,
				 unsigned int width, unsigned int height, unsigned int stride)
{
	const struct color_rgb24 colors_top[] = {
		MAKE_RGB24(rgb, 192, 192, 192),	/* grey */
		MAKE_RGB24(rgb, 192, 192, 0),	/* yellow */
		MAKE_RGB24(rgb, 0, 192, 192),	/* cyan */
		MAKE_RGB24(rgb, 0, 192, 0),	/* green */
		MAKE_RGB24(rgb, 192, 0, 192),	/* magenta */
		MAKE_RGB24(rgb, 192, 0, 0),	/* red */
		MAKE_RGB24(rgb, 0, 0, 192),	/* blue */
	};
	const struct color_rgb24 colors_middle[] = {
		MAKE_RGB24(rgb, 0, 0, 192),	/* blue */
		MAKE_RGB24(rgb, 19, 19, 19),	/* black */
		MAKE_RGB24(rgb, 192, 0, 192),	/* magenta */
		MAKE_RGB24(rgb, 19, 19, 19),	/* black */
		MAKE_RGB24(rgb, 0, 192, 192),	/* cyan */
		MAKE_RGB24(rgb, 19, 19, 19),	/* black */
		MAKE_RGB24(rgb, 192, 192, 192),	/* grey */
	};
	const struct color_rgb24 colors_bottom[] = {
		MAKE_RGB24(rgb, 0, 33, 76),	/* in-phase */
		MAKE_RGB24(rgb, 255, 255, 255),	/* super white */
		MAKE_RGB24(rgb, 50, 0, 106),	/* quadrature */
		MAKE_RGB24(rgb, 19, 19, 19),	/* black */
		MAKE_RGB24(rgb, 9, 9, 9),	/* 3.5% */
		MAKE_RGB24(rgb, 19, 19, 19),	/* 7.5% */
		MAKE_RGB24(rgb, 29, 29, 29),	/* 11.5% */
		MAKE_RGB24(rgb, 19, 19, 19),	/* black */
	};
	unsigned int x;
	unsigned int y;

	if (width < 8)
		return;

	for (y = 0; y < height * 6 / 9; ++y) {
		for (x = 0; x < width; ++x)
			((struct color_rgb24 *)mem)[x] =
				colors_top[x * 7 / width];
		mem += stride;
	}

	for (; y < height * 7 / 9; ++y) {
		for (x = 0; x < width; ++x)
			((struct color_rgb24 *)mem)[x] =
				colors_middle[x * 7 / width];
		mem += stride;
	}

	for (; y < height; ++y) {
		for (x = 0; x < width * 5 / 7; ++x)
			((struct color_rgb24 *)mem)[x] =
				colors_bottom[x * 4 / (width * 5 / 7)];
		for (; x < width * 6 / 7; ++x)
			((struct color_rgb24 *)mem)[x] =
				colors_bottom[(x - width * 5 / 7) * 3
							  / (width / 7) + 4];
		for (; x < width; ++x)
			((struct color_rgb24 *)mem)[x] = colors_bottom[7];
		mem += stride;
	}
}

static void
fill_smpte_rgb32(const struct rgb_info *rgb, unsigned char *mem,
				 unsigned int width, unsigned int height, unsigned int stride)
{
	const uint32_t colors_top[] = {
		MAKE_RGBA(rgb, 192, 192, 192, 255),	/* grey */
		MAKE_RGBA(rgb, 192, 192, 0, 255),	/* yellow */
		MAKE_RGBA(rgb, 0, 192, 192, 255),	/* cyan */
		MAKE_RGBA(rgb, 0, 192, 0, 255),		/* green */
		MAKE_RGBA(rgb, 192, 0, 192, 255),	/* magenta */
		MAKE_RGBA(rgb, 192, 0, 0, 255),		/* red */
		MAKE_RGBA(rgb, 0, 0, 192, 255),		/* blue */
	};
	const uint32_t colors_middle[] = {
		MAKE_RGBA(rgb, 0, 0, 192, 255),		/* blue */
		MAKE_RGBA(rgb, 19, 19, 19, 255),	/* black */
		MAKE_RGBA(rgb, 192, 0, 192, 255),	/* magenta */
		MAKE_RGBA(rgb, 19, 19, 19, 255),	/* black */
		MAKE_RGBA(rgb, 0, 192, 192, 255),	/* cyan */
		MAKE_RGBA(rgb, 19, 19, 19, 255),	/* black */
		MAKE_RGBA(rgb, 192, 192, 192, 255),	/* grey */
	};
	const uint32_t colors_bottom[] = {
		MAKE_RGBA(rgb, 0, 33, 76, 255),		/* in-phase */
		MAKE_RGBA(rgb, 255, 255, 255, 255),	/* super white */
		MAKE_RGBA(rgb, 50, 0, 106, 255),	/* quadrature */
		MAKE_RGBA(rgb, 19, 19, 19, 255),	/* black */
		MAKE_RGBA(rgb, 9, 9, 9, 255),		/* 3.5% */
		MAKE_RGBA(rgb, 19, 19, 19, 255),	/* 7.5% */
		MAKE_RGBA(rgb, 29, 29, 29, 255),	/* 11.5% */
		MAKE_RGBA(rgb, 19, 19, 19, 255),	/* black */
	};
	unsigned int x;
	unsigned int y;

	if (width < 8)
		return;

	for (y = 0; y < height * 6 / 9; ++y) {
		for (x = 0; x < width; ++x)
			((uint32_t *)mem)[x] = colors_top[x * 7 / width];
		mem += stride;
	}

	for (; y < height * 7 / 9; ++y) {
		for (x = 0; x < width; ++x)
			((uint32_t *)mem)[x] = colors_middle[x * 7 / width];
		mem += stride;
	}

	for (; y < height; ++y) {
		for (x = 0; x < width * 5 / 7; ++x)
			((uint32_t *)mem)[x] =
				colors_bottom[x * 4 / (width * 5 / 7)];
		for (; x < width * 6 / 7; ++x)
			((uint32_t *)mem)[x] =
				colors_bottom[(x - width * 5 / 7) * 3
							  / (width / 7) + 4];
		for (; x < width; ++x) {
			((uint32_t *)mem)[x] = (rand_r(&rand_seed) % 2) ? MAKE_RGBA(rgb, 255, 255, 255, 255) : MAKE_RGBA(rgb, 0, 0, 0, 255);
		}
		mem += stride;
	}
}

static void
fill_smpte(const struct format_info *info, void *planes[3], unsigned int width,
		   unsigned int height, unsigned int stride)
{
	unsigned char *u, *v;

	switch (info->format) {
	case TBM_FORMAT_UYVY:
	case TBM_FORMAT_VYUY:
	case TBM_FORMAT_YUYV:
	case TBM_FORMAT_YVYU:
		return fill_smpte_yuv_packed(&info->yuv, planes[0], width,
									 height, stride);

	case TBM_FORMAT_NV12:
	case TBM_FORMAT_NV21:
	case TBM_FORMAT_NV16:
	case TBM_FORMAT_NV61:
		u = info->yuv.order & YUV_YCbCr ? planes[1] : planes[1] + 1;
		v = info->yuv.order & YUV_YCrCb ? planes[1] : planes[1] + 1;
		return fill_smpte_yuv_planar(&info->yuv, planes[0], u, v,
									 width, height, stride);

	case TBM_FORMAT_YUV420:
		return fill_smpte_yuv_planar(&info->yuv, planes[0], planes[1],
									 planes[2], width, height, stride);

	case TBM_FORMAT_YVU420:
		return fill_smpte_yuv_planar(&info->yuv, planes[0], planes[2],
									 planes[1], width, height, stride);

	case TBM_FORMAT_ARGB4444:
	case TBM_FORMAT_XRGB4444:
	case TBM_FORMAT_ABGR4444:
	case TBM_FORMAT_XBGR4444:
	case TBM_FORMAT_RGBA4444:
	case TBM_FORMAT_RGBX4444:
	case TBM_FORMAT_BGRA4444:
	case TBM_FORMAT_BGRX4444:
	case TBM_FORMAT_RGB565:
	case TBM_FORMAT_BGR565:
	case TBM_FORMAT_ARGB1555:
	case TBM_FORMAT_XRGB1555:
	case TBM_FORMAT_ABGR1555:
	case TBM_FORMAT_XBGR1555:
	case TBM_FORMAT_RGBA5551:
	case TBM_FORMAT_RGBX5551:
	case TBM_FORMAT_BGRA5551:
	case TBM_FORMAT_BGRX5551:
		return fill_smpte_rgb16(&info->rgb, planes[0],
								width, height, stride);

	case TBM_FORMAT_BGR888:
	case TBM_FORMAT_RGB888:
		return fill_smpte_rgb24(&info->rgb, planes[0],
								width, height, stride);
	case TBM_FORMAT_ARGB8888:
	case TBM_FORMAT_XRGB8888:
	case TBM_FORMAT_ABGR8888:
	case TBM_FORMAT_XBGR8888:
	case TBM_FORMAT_RGBA8888:
	case TBM_FORMAT_RGBX8888:
	case TBM_FORMAT_BGRA8888:
	case TBM_FORMAT_BGRX8888:
	case TBM_FORMAT_ARGB2101010:
	case TBM_FORMAT_XRGB2101010:
	case TBM_FORMAT_ABGR2101010:
	case TBM_FORMAT_XBGR2101010:
	case TBM_FORMAT_RGBA1010102:
	case TBM_FORMAT_RGBX1010102:
	case TBM_FORMAT_BGRA1010102:
	case TBM_FORMAT_BGRX1010102:
		return fill_smpte_rgb32(&info->rgb, planes[0],
								width, height, stride);
	}
}

static void
fill_tiles_yuv_planar(const struct format_info *info,
					  unsigned char *y_mem, unsigned char *u_mem,
					  unsigned char *v_mem, unsigned int width,
					  unsigned int height, unsigned int stride)
{
	const struct yuv_info *yuv = &info->yuv;
	unsigned int cs = yuv->chroma_stride;
	unsigned int xsub = yuv->xsub;
	unsigned int ysub = yuv->ysub;
	unsigned int x;
	unsigned int y;

	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			div_t d = div(x + y, width);
			uint32_t rgb32 = 0x00130502 * (d.quot >> 6)
							 + 0x000a1120 * (d.rem >> 6);
			struct color_yuv color =
				MAKE_YUV_601((rgb32 >> 16) & 0xff,
							 (rgb32 >> 8) & 0xff, rgb32 & 0xff);

			y_mem[x] = color.y;
			u_mem[x / xsub * cs] = color.u;
			v_mem[x / xsub * cs] = color.v;
		}

		y_mem += stride;
		if ((y + 1) % ysub == 0) {
			u_mem += stride * cs / xsub;
			v_mem += stride * cs / xsub;
		}
	}
}

static void
fill_tiles_yuv_packed(const struct format_info *info, unsigned char *mem,
					  unsigned int width, unsigned int height,
					  unsigned int stride)
{
	const struct yuv_info *yuv = &info->yuv;
	unsigned char *y_mem = (yuv->order & YUV_YC) ? mem : mem + 1;
	unsigned char *c_mem = (yuv->order & YUV_CY) ? mem : mem + 1;
	unsigned int u = (yuv->order & YUV_YCrCb) ? 2 : 0;
	unsigned int v = (yuv->order & YUV_YCbCr) ? 2 : 0;
	unsigned int x;
	unsigned int y;

	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; x += 2) {
			div_t d = div(x + y, width);
			uint32_t rgb32 = 0x00130502 * (d.quot >> 6)
							 + 0x000a1120 * (d.rem >> 6);
			struct color_yuv color =
				MAKE_YUV_601((rgb32 >> 16) & 0xff,
							 (rgb32 >> 8) & 0xff, rgb32 & 0xff);

			y_mem[2 * x] = color.y;
			c_mem[2 * x + u] = color.u;
			y_mem[2 * x + 2] = color.y;
			c_mem[2 * x + v] = color.v;
		}

		y_mem += stride;
		c_mem += stride;
	}
}

static void
fill_tiles_rgb16(const struct format_info *info, unsigned char *mem,
				 unsigned int width, unsigned int height, unsigned int stride)
{
	const struct rgb_info *rgb = &info->rgb;
	unsigned int x, y;

	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			div_t d = div(x + y, width);
			uint32_t rgb32 = 0x00130502 * (d.quot >> 6)
							 + 0x000a1120 * (d.rem >> 6);
			uint16_t color =
				MAKE_RGBA(rgb, (rgb32 >> 16) & 0xff,
						  (rgb32 >> 8) & 0xff, rgb32 & 0xff,
						  255);

			((uint16_t *)mem)[x] = color;
		}
		mem += stride;
	}
}

static void
fill_tiles_rgb24(const struct format_info *info, unsigned char *mem,
				 unsigned int width, unsigned int height, unsigned int stride)
{
	const struct rgb_info *rgb = &info->rgb;
	unsigned int x, y;

	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			div_t d = div(x + y, width);
			uint32_t rgb32 = 0x00130502 * (d.quot >> 6)
							 + 0x000a1120 * (d.rem >> 6);
			struct color_rgb24 color =
				MAKE_RGB24(rgb, (rgb32 >> 16) & 0xff,
						   (rgb32 >> 8) & 0xff, rgb32 & 0xff);

			((struct color_rgb24 *)mem)[x] = color;
		}
		mem += stride;
	}
}

static void
fill_tiles_rgb32(const struct format_info *info, unsigned char *mem,
				 unsigned int width, unsigned int height, unsigned int stride)
{
	const struct rgb_info *rgb = &info->rgb;
	unsigned int x, y;

	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			div_t d = div(x + y, width);
			uint32_t rgb32 = 0x00130502 * (d.quot >> 6)
							 + 0x000a1120 * (d.rem >> 6);
			uint32_t color =
				MAKE_RGBA(rgb, (rgb32 >> 16) & 0xff,
						  (rgb32 >> 8) & 0xff, rgb32 & 0xff,
						  255);

			((uint32_t *)mem)[x] = color;
		}
		mem += stride;
	}
}

static void
fill_tiles(const struct format_info *info, void *planes[3], unsigned int width,
		   unsigned int height, unsigned int stride)
{
	unsigned char *u, *v;

	switch (info->format) {
	case TBM_FORMAT_UYVY:
	case TBM_FORMAT_VYUY:
	case TBM_FORMAT_YUYV:
	case TBM_FORMAT_YVYU:
		return fill_tiles_yuv_packed(info, planes[0],
									 width, height, stride);

	case TBM_FORMAT_NV12:
	case TBM_FORMAT_NV21:
	case TBM_FORMAT_NV16:
	case TBM_FORMAT_NV61:
		u = info->yuv.order & YUV_YCbCr ? planes[1] : planes[1] + 1;
		v = info->yuv.order & YUV_YCrCb ? planes[1] : planes[1] + 1;
		return fill_tiles_yuv_planar(info, planes[0], u, v,
									 width, height, stride);

	case TBM_FORMAT_YUV420:
		return fill_tiles_yuv_planar(info, planes[0], planes[1],
									 planes[2], width, height, stride);

	case TBM_FORMAT_YVU420:
		return fill_tiles_yuv_planar(info, planes[0], planes[2],
									 planes[1], width, height, stride);

	case TBM_FORMAT_ARGB4444:
	case TBM_FORMAT_XRGB4444:
	case TBM_FORMAT_ABGR4444:
	case TBM_FORMAT_XBGR4444:
	case TBM_FORMAT_RGBA4444:
	case TBM_FORMAT_RGBX4444:
	case TBM_FORMAT_BGRA4444:
	case TBM_FORMAT_BGRX4444:
	case TBM_FORMAT_RGB565:
	case TBM_FORMAT_BGR565:
	case TBM_FORMAT_ARGB1555:
	case TBM_FORMAT_XRGB1555:
	case TBM_FORMAT_ABGR1555:
	case TBM_FORMAT_XBGR1555:
	case TBM_FORMAT_RGBA5551:
	case TBM_FORMAT_RGBX5551:
	case TBM_FORMAT_BGRA5551:
	case TBM_FORMAT_BGRX5551:
		return fill_tiles_rgb16(info, planes[0],
								width, height, stride);

	case TBM_FORMAT_BGR888:
	case TBM_FORMAT_RGB888:
		return fill_tiles_rgb24(info, planes[0],
								width, height, stride);
	case TBM_FORMAT_ARGB8888:
	case TBM_FORMAT_XRGB8888:
	case TBM_FORMAT_ABGR8888:
	case TBM_FORMAT_XBGR8888:
	case TBM_FORMAT_RGBA8888:
	case TBM_FORMAT_RGBX8888:
	case TBM_FORMAT_BGRA8888:
	case TBM_FORMAT_BGRX8888:
	case TBM_FORMAT_ARGB2101010:
	case TBM_FORMAT_XRGB2101010:
	case TBM_FORMAT_ABGR2101010:
	case TBM_FORMAT_XBGR2101010:
	case TBM_FORMAT_RGBA1010102:
	case TBM_FORMAT_RGBX1010102:
	case TBM_FORMAT_BGRA1010102:
	case TBM_FORMAT_BGRX1010102:
		return fill_tiles_rgb32(info, planes[0],
								width, height, stride);
	}
}

static void
fill_plain(const struct format_info *info, void *planes[3], unsigned int width,
		   unsigned int height, unsigned int stride)
{
	memset(planes[0], 0x77, stride * height);
}

/*
 * fill_pattern - Fill a buffer with a test pattern
 * @format: Pixel format
 * @pattern: Test pattern
 * @buffer: Buffer memory
 * @width: Width in pixels
 * @height: Height in pixels
 * @stride: Line stride (pitch) in bytes
 *
 * Fill the buffer with the test pattern specified by the pattern parameter.
 * Supported formats vary depending on the selected pattern.
 */
static void
fill_pattern(unsigned int format, enum fill_pattern pattern, void *planes[3],
			 unsigned int width, unsigned int height, unsigned int stride)
{
	const struct format_info *info = NULL;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(format_info); ++i) {
		if (format_info[i].format == format) {
			info = &format_info[i];
			break;
		}
	}

	if (info == NULL)
		return;

	switch (pattern) {
	case PATTERN_TILES:
		return fill_tiles(info, planes, width, height, stride);

	case PATTERN_SMPTE:
		return fill_smpte(info, planes, width, height, stride);

	case PATTERN_PLAIN:
		return fill_plain(info, planes, width, height, stride);

	default:
		printf("Error: unsupported test pattern %u.\n", pattern);
		break;
	}
}

void
tdm_test_buffer_fill(tbm_surface_h buffer, int pattern)
{
	tbm_surface_info_s info;
	void *plane[3];
	int ret;

	if (rand_seed == 0)
		rand_seed = time(NULL);

	ret = tbm_surface_map(buffer, TBM_OPTION_WRITE, &info);
	TDM_EXIT_IF_FAIL(ret == 0);

	plane[0] = info.planes[0].ptr;
	plane[1] = info.planes[1].ptr;
	plane[2] = info.planes[2].ptr;
	fill_pattern(info.format, pattern, plane, info.width, info.height, info.planes[0].stride);
	tbm_surface_unmap(buffer);
}
