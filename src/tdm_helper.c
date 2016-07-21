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

#include <png.h>
#include <string.h>
#include <tbm_surface.h>
#include <tbm_surface_internal.h>
#include <string.h>
#include <time.h>
#include <pixman.h>

#include "tdm.h"
#include "tdm_private.h"
#include "tdm_helper.h"

#define PNG_DEPTH 8

static const char *file_exts[2] = {"png", "yuv"};

int tdm_dump_enable;

INTERN unsigned long
tdm_helper_get_time_in_millis(void)
{
	struct timespec tp;

	if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
		return (tp.tv_sec * 1000) + (tp.tv_nsec / 1000000L);

	return 0;
}

INTERN unsigned long
tdm_helper_get_time_in_micros(void)
{
	struct timespec tp;

	if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
		return (tp.tv_sec * 1000000) + (tp.tv_nsec / 1000L);

	return 0;
}

static void
_tdm_helper_dump_raw(const char *file, void *data1, int size1, void *data2,
					 int size2, void *data3, int size3)
{
	unsigned int *blocks;
	FILE *fp = fopen(file, "w+");
	TDM_RETURN_IF_FAIL(fp != NULL);

	blocks = (unsigned int *)data1;
	fwrite(blocks, 1, size1, fp);

	if (size2 > 0) {
		blocks = (unsigned int *)data2;
		fwrite(blocks, 1, size2, fp);
	}

	if (size3 > 0) {
		blocks = (unsigned int *)data3;
		fwrite(blocks, 1, size3, fp);
	}

	fclose(fp);
}

static void
_tdm_helper_dump_png(const char *file, const void *data, int width,
					 int height)
{
	FILE *fp = fopen(file, "wb");
	TDM_RETURN_IF_FAIL(fp != NULL);

	png_structp pPngStruct =
		png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!pPngStruct) {
		fclose(fp);
		return;
	}

	png_infop pPngInfo = png_create_info_struct(pPngStruct);
	if (!pPngInfo) {
		png_destroy_write_struct(&pPngStruct, NULL);
		fclose(fp);
		return;
	}

	png_init_io(pPngStruct, fp);
	png_set_IHDR(pPngStruct,
				 pPngInfo,
				 width,
				 height,
				 PNG_DEPTH,
				 PNG_COLOR_TYPE_RGBA,
				 PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_set_bgr(pPngStruct);
	png_write_info(pPngStruct, pPngInfo);

	const int pixel_size = 4;	// RGBA
	png_bytep *row_pointers =
		png_malloc(pPngStruct, height * sizeof(png_byte *));

	unsigned int *blocks = (unsigned int *)data;
	int y = 0;
	int x = 0;

	for (; y < height; ++y) {
		png_bytep row =
			png_malloc(pPngStruct, sizeof(png_byte) * width * pixel_size);
		row_pointers[y] = (png_bytep)row;
		for (x = 0; x < width; ++x) {
			unsigned int curBlock = blocks[y * width + x];
			row[x * pixel_size] = (curBlock & 0xFF);
			row[1 + x * pixel_size] = (curBlock >> 8) & 0xFF;
			row[2 + x * pixel_size] = (curBlock >> 16) & 0xFF;
			row[3 + x * pixel_size] = (curBlock >> 24) & 0xFF;
		}
	}

	png_write_image(pPngStruct, row_pointers);
	png_write_end(pPngStruct, pPngInfo);

	for (y = 0; y < height; y++)
		png_free(pPngStruct, row_pointers[y]);
	png_free(pPngStruct, row_pointers);

	png_destroy_write_struct(&pPngStruct, &pPngInfo);

	fclose(fp);
}

INTERN void
tdm_helper_dump_buffer_str(tbm_surface_h buffer, const char *str)
{
	tbm_surface_info_s info;
	const char *dir = "/tmp/dump-tdm";
	const char *ext;
	char file[TDM_PATH_LEN];
	int ret, bw;

	TDM_RETURN_IF_FAIL(buffer != NULL);
	TDM_RETURN_IF_FAIL(str != NULL);

	ret = tbm_surface_get_info(buffer, &info);
	TDM_RETURN_IF_FAIL(ret == TBM_SURFACE_ERROR_NONE);

	if (info.format == TBM_FORMAT_ARGB8888 || info.format == TBM_FORMAT_XRGB8888) {
		ext = file_exts[0];
		bw = info.planes[0].stride >> 2;
	} else {
		ext = file_exts[1];
		bw = info.planes[0].stride;
	}

	snprintf(file, TDM_PATH_LEN, "%s/%c%c%c%c_%dx%d_%s.%s",
			 dir, FOURCC_STR(info.format), bw, info.height, str, ext);

	tdm_helper_dump_buffer(buffer, file);
}

EXTERN void
tdm_helper_dump_buffer(tbm_surface_h buffer, const char *file)
{
	tbm_surface_info_s info;
	int len, ret;
	const char *ext;

	TDM_RETURN_IF_FAIL(buffer != NULL);
	TDM_RETURN_IF_FAIL(file != NULL);

	ret = tbm_surface_map(buffer, TBM_OPTION_READ, &info);
	TDM_RETURN_IF_FAIL(ret == TBM_SURFACE_ERROR_NONE);

	len = strnlen(file, 1024);
	if (info.format == TBM_FORMAT_ARGB8888 || info.format == TBM_FORMAT_XRGB8888)
		ext = file_exts[0];
	else
		ext = file_exts[1];

	if (strncmp(file + (len - 3), ext, 3)) {
		TDM_ERR("can't dump to '%s' file", file + (len - 3));
		tbm_surface_unmap(buffer);
		return;
	}

	switch (info.format) {
	case TBM_FORMAT_ARGB8888:
	case TBM_FORMAT_XRGB8888:
		_tdm_helper_dump_png(file, info.planes[0].ptr,
							 info.planes[0].stride >> 2, info.height);
		break;
	case TBM_FORMAT_YVU420:
	case TBM_FORMAT_YUV420:
		_tdm_helper_dump_raw(file,
							 info.planes[0].ptr,
							 info.planes[0].stride * info.height,
							 info.planes[1].ptr,
							 info.planes[1].stride * (info.height >> 1),
							 info.planes[2].ptr,
							 info.planes[2].stride * (info.height >> 1));
		break;
	case TBM_FORMAT_NV12:
	case TBM_FORMAT_NV21:
		_tdm_helper_dump_raw(file,
							 info.planes[0].ptr,
							 info.planes[0].stride * info.height,
							 info.planes[1].ptr,
							 info.planes[1].stride * (info.height >> 1), NULL,
							 0);
		break;
	case TBM_FORMAT_YUYV:
	case TBM_FORMAT_UYVY:
		_tdm_helper_dump_raw(file,
							 info.planes[0].ptr,
							 info.planes[0].stride * info.height, NULL, 0,
							 NULL, 0);
		break;
	default:
		TDM_ERR("can't dump %c%c%c%c buffer", FOURCC_STR(info.format));
		tbm_surface_unmap(buffer);
		return;
	}

	tbm_surface_unmap(buffer);

	TDM_INFO("dump %s", file);
}

EXTERN void
tdm_helper_clear_buffer(tbm_surface_h buffer)
{
	tbm_surface_info_s info;
	int ret;

	TDM_RETURN_IF_FAIL(buffer != NULL);

	ret = tbm_surface_map(buffer, TBM_OPTION_READ, &info);
	TDM_RETURN_IF_FAIL(ret == TBM_SURFACE_ERROR_NONE);

	switch (info.format) {
	case TBM_FORMAT_ARGB8888:
	case TBM_FORMAT_XRGB8888:
		memset(info.planes[0].ptr, 0, info.planes[0].stride * info.height);
		break;
	case TBM_FORMAT_YVU420:
	case TBM_FORMAT_YUV420:
		memset((char*)info.planes[0].ptr, 0x10, info.planes[0].stride * info.height);
		memset((char*)info.planes[1].ptr, 0x80, info.planes[1].stride * (info.height >> 1));
		memset((char*)info.planes[2].ptr, 0x80, info.planes[2].stride * (info.height >> 1));
		break;
	case TBM_FORMAT_NV12:
	case TBM_FORMAT_NV21:
		memset((char*)info.planes[0].ptr, 0x10, info.planes[0].stride * info.height);
		memset((char*)info.planes[1].ptr, 0x80, info.planes[1].stride * (info.height >> 1));
		break;
	case TBM_FORMAT_YUYV: {
		int *ibuf = (int*)info.planes[0].ptr;
		int i, size = info.planes[0].stride * info.height / 4;

		for (i = 0 ; i < size ; i++)
			ibuf[i] = 0x10801080;
	}
	break;
	case TBM_FORMAT_UYVY: {
		int *ibuf = (int*)info.planes[0].ptr;
		int i, size = info.planes[0].stride * info.height / 4;

		for (i = 0 ; i < size ; i++)
			ibuf[i] = 0x80108010; /* YUYV -> 0xVYUY */
	}
	break;
	default:
		TDM_ERR("can't clear %c%c%c%c buffer", FOURCC_STR(info.format));
		break;
	}

	tbm_surface_unmap(buffer);
}

EXTERN int
tdm_helper_get_fd(const char *env)
{
	const char *value;
	int fd, newfd, flags, ret;

	value = (const char*)getenv(env);
	if (!value)
		return -1;

	ret = sscanf(value, "%d", &fd);
	if (ret < 0) {
		TDM_ERR("sscanf failed: %m");
		return -1;
	}

	flags = fcntl(fd, F_GETFD);
	if (flags == -1) {
		TDM_ERR("fcntl failed: %m");
		return -1;
	}

	newfd = dup(fd);
	if (newfd < 0) {
		TDM_ERR("dup failed: %m");
		return -1;
	}

	TDM_INFO("%s: fd(%d) newfd(%d)", env, fd, newfd);

	fcntl(newfd, F_SETFD, flags | FD_CLOEXEC);

	return newfd;
}

EXTERN void
tdm_helper_set_fd(const char *env, int fd)
{
	char buf[32];
	int ret;

	snprintf(buf, sizeof(buf), "%d", fd);

	ret = setenv(env, (const char*)buf, 1);
	if (ret) {
		TDM_ERR("setenv failed: %m");
		return;
	}

	if (fd >= 0)
		TDM_INFO("%s: fd(%d)", env, fd);
}

EXTERN void
tdm_helper_dump_start(char *dumppath, int *count)
{
	if (dumppath == NULL || count == NULL) {
		TDM_DBG("tdm_helper_dump dumppath or count is null.");
		return;
	}

	tdm_dump_enable = 1;

	TDM_DBG("tdm_helper_dump start.(path : %s)", dumppath);
}

EXTERN void
tdm_helper_dump_stop(void)
{
	tdm_dump_enable = 0;

	TDM_DBG("tdm_helper_dump stop.");
}

static pixman_format_code_t
_tdm_helper_pixman_format_get(tbm_format format)
{
	switch (format) {
	case TBM_FORMAT_ARGB8888:
		return PIXMAN_a8r8g8b8;
	case TBM_FORMAT_XRGB8888:
		return PIXMAN_x8r8g8b8;
	default:
		return 0;
	}

	return 0;
}

static tdm_error
_tdm_helper_buffer_convert(tbm_surface_h srcbuf, tbm_surface_h dstbuf,
						   int dx, int dy, int dw, int dh, int count)
{
	pixman_image_t *src_img = NULL, *dst_img = NULL;
	pixman_format_code_t src_format, dst_format;
	pixman_transform_t t;
	struct pixman_f_transform ft;
	pixman_op_t op;
	tbm_surface_info_s src_info = {0, };
	tbm_surface_info_s dst_info = {0, };
	int stride, width;
	double scale_x, scale_y;

	TDM_RETURN_VAL_IF_FAIL(srcbuf != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(dstbuf != NULL, TDM_ERROR_INVALID_PARAMETER);

	if (tbm_surface_map(srcbuf, TBM_SURF_OPTION_READ, &src_info)
			!= TBM_SURFACE_ERROR_NONE) {
		TDM_ERR("cannot mmap srcbuf\n");
		return TDM_ERROR_OPERATION_FAILED;
	}

	if (tbm_surface_map(dstbuf, TBM_SURF_OPTION_WRITE, &dst_info)
			!= TBM_SURFACE_ERROR_NONE) {
		TDM_ERR("cannot mmap dstbuf\n");
		tbm_surface_unmap(srcbuf);
		return TDM_ERROR_OPERATION_FAILED;
	}
	TDM_GOTO_IF_FAIL(src_info.num_planes == 1, cant_convert);
	TDM_GOTO_IF_FAIL(dst_info.num_planes == 1, cant_convert);

	/* src */
	src_format = _tdm_helper_pixman_format_get(src_info.format);
	TDM_GOTO_IF_FAIL(src_format > 0, cant_convert);

	width = src_info.planes[0].stride / 4;
	stride = src_info.planes[0].stride;
	src_img = pixman_image_create_bits(src_format, width, src_info.height,
									   (uint32_t*)src_info.planes[0].ptr, stride);
	TDM_GOTO_IF_FAIL(src_img != NULL, cant_convert);

	/* dst */
	dst_format = _tdm_helper_pixman_format_get(dst_info.format);
	TDM_GOTO_IF_FAIL(dst_format > 0, cant_convert);

	width = dst_info.planes[0].stride / 4;
	stride = dst_info.planes[0].stride;
	dst_img = pixman_image_create_bits(dst_format, width, dst_info.height,
									   (uint32_t*)dst_info.planes[0].ptr, stride);
	TDM_GOTO_IF_FAIL(dst_img != NULL, cant_convert);

	pixman_f_transform_init_identity(&ft);

	scale_x = (double)src_info.width / dw;
	scale_y = (double)src_info.height / dh;

	pixman_f_transform_scale(&ft, NULL, scale_x, scale_y);
	pixman_f_transform_translate(&ft, NULL, 0, 0);
	pixman_transform_from_pixman_f_transform(&t, &ft);
	pixman_image_set_transform(src_img, &t);

	if (count == 0)
		op = PIXMAN_OP_SRC;
	else
		op = PIXMAN_OP_OVER;

	pixman_image_composite(op, src_img, NULL, dst_img,
						   0, 0, 0, 0, dx, dy, dw, dh);

	if (src_img)
		pixman_image_unref(src_img);
	if (dst_img)
		pixman_image_unref(dst_img);

	tbm_surface_unmap(srcbuf);
	tbm_surface_unmap(dstbuf);

	return TDM_ERROR_NONE;

cant_convert:
	if (src_img)
		pixman_image_unref(src_img);

	tbm_surface_unmap(srcbuf);
	tbm_surface_unmap(dstbuf);

	return TDM_ERROR_OPERATION_FAILED;
}

EXTERN tdm_error
tdm_helper_capture_output(tdm_output *output, tbm_surface_h dst_buffer,
						  int x, int y, int w, int h,
						  tdm_helper_capture_handler func, void *data)
{
	tbm_surface_h surface;
	tdm_error err;
	int i, count, first = 0;

	TDM_RETURN_VAL_IF_FAIL(output != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(dst_buffer != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(x >= 0, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(y >= 0, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(w >= 0, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(h >= 0, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(func != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(data != NULL, TDM_ERROR_INVALID_PARAMETER);

	err = tdm_output_get_layer_count(output, &count);
	if (err != TDM_ERROR_NONE) {
		TDM_ERR("tdm_output_get_layer_count fail(%d)\n", err);
		return TDM_ERROR_OPERATION_FAILED;
	}
	if (count <= 0) {
		TDM_ERR("tdm_output_get_layer_count err(%d, %d)\n", err, count);
		return TDM_ERROR_BAD_MODULE;
	}

	for (i = count - 1; i >= 0; i--) {
		tdm_layer *layer = tdm_output_get_layer(output, i, NULL);

		surface = tdm_layer_get_displaying_buffer(layer, &err);
		if (err != TDM_ERROR_NONE)
			continue;

		err = _tdm_helper_buffer_convert(surface, dst_buffer, x, y, w, h, first++);
		if (err != TDM_ERROR_NONE)
			TDM_DBG("convert fail %d-layer buffer\n", i);
		else
			TDM_DBG("convert success %d-layer buffer\n", i);
	}

	func(dst_buffer, data);

	return TDM_ERROR_NONE;
}

EXTERN void
tdm_helper_get_display_information(tdm_display *dpy, char *reply, int *len)
{
	tdm_private_display *private_display;
	tdm_backend_module *module_data;
	tdm_func_output *func_output;
	tdm_func_layer *func_layer;
	tdm_private_output *private_output = NULL;
	tdm_private_layer *private_layer = NULL;
	tdm_private_pp *private_pp = NULL;
	tdm_private_capture *private_capture = NULL;
	tdm_error ret;
	int i;

	TDM_DBG_RETURN_IF_FAIL(dpy != NULL);

	private_display = dpy;
	func_output = &private_display->func_output;
	func_layer = &private_display->func_layer;
	_pthread_mutex_lock(&private_display->lock);

	/* module information */
	module_data = private_display->module_data;
	TDM_SNPRINTF(reply, len, "[TDM backend information]\n");
	TDM_SNPRINTF(reply, len, "name: %s\n", module_data->name);
	TDM_SNPRINTF(reply, len, "vendor: %s\n", module_data->vendor);
	TDM_SNPRINTF(reply, len, "version: %d.%d\n\n",
				 (int)TDM_BACKEND_GET_ABI_MAJOR(module_data->abi_version),
				 (int)TDM_BACKEND_GET_ABI_MINOR(module_data->abi_version));

	/* output information */
	TDM_SNPRINTF(reply, len, "[Output information]\n");
	TDM_SNPRINTF(reply, len, "--------------------------------------------------------------------------------------------\n");
	TDM_SNPRINTF(reply, len, "idx   maker   model   name   type   status   dpms   subpixel   align_w   min   max   phy(mm)\n");
	TDM_SNPRINTF(reply, len, "--------------------------------------------------------------------------------------------\n");
	LIST_FOR_EACH_ENTRY(private_output, &private_display->output_list, link) {
		TDM_SNPRINTF(reply, len, "%d   %s   %s   %s   %s   %s   %s   %d   %d   %dx%d   %dx%d   %dx%d\n",
					 private_output->index, private_output->caps.maker,
					 private_output->caps.model, private_output->caps.name,
					 tdm_conn_str(private_output->caps.type),
					 tdm_status_str(private_output->caps.status),
					 tdm_dpms_str(private_output->current_dpms_value),
					 private_output->caps.subpixel,
					 private_output->caps.preferred_align,
					 private_output->caps.min_w, private_output->caps.min_h,
					 private_output->caps.max_w, private_output->caps.max_h,
					 private_output->caps.mmWidth, private_output->caps.mmHeight);

		TDM_SNPRINTF(reply, len, "\t%d modes:\n", private_output->caps.mode_count);

		if (private_output->caps.mode_count > 0) {
			const tdm_output_mode *current_mode = NULL;

			TDM_DBG_GOTO_IF_FAIL(func_output->output_get_mode, unlock);
			ret = func_output->output_get_mode(private_output->output_backend, &current_mode);
			TDM_DBG_GOTO_IF_FAIL(ret == TDM_ERROR_NONE, unlock);

			TDM_SNPRINTF(reply, len, "\t\t name refresh (Hz) hdisp hss hse htot vdisp vss vse vtot\n");
			for (i = 0; i < private_output->caps.mode_count; i++) {
				char *current = (current_mode == private_output->caps.modes + i) ? "*" : " ";
				TDM_SNPRINTF(reply, len, "\t\t%s%s %d %d %d %d %d %d %d %d %d ",
							 current,
							 private_output->caps.modes[i].name,
							 private_output->caps.modes[i].vrefresh,
							 private_output->caps.modes[i].hdisplay,
							 private_output->caps.modes[i].hsync_start,
							 private_output->caps.modes[i].hsync_end,
							 private_output->caps.modes[i].htotal,
							 private_output->caps.modes[i].vdisplay,
							 private_output->caps.modes[i].vsync_start,
							 private_output->caps.modes[i].vsync_end,
							 private_output->caps.modes[i].vtotal);
				tdm_mode_flag_str(private_output->caps.modes[i].flags, &reply, len);
				TDM_SNPRINTF(reply, len, " ");
				tdm_mode_type_str(private_output->caps.modes[i].type, &reply, len);
				TDM_SNPRINTF(reply, len, "\n");
			}
		}

		TDM_SNPRINTF(reply, len, "\t%d properties:\n", private_output->caps.prop_count);
		if (private_output->caps.prop_count > 0) {
			TDM_SNPRINTF(reply, len, "\t\tname\tidx\tvalue\n");
			for (i = 0; i < private_output->caps.prop_count; i++) {
				tdm_value value;
				TDM_DBG_GOTO_IF_FAIL(func_output->output_get_property, unlock);
				ret = func_output->output_get_property(private_output->output_backend,
													   private_output->caps.props[i].id,
													   &value);
				TDM_DBG_GOTO_IF_FAIL(ret == TDM_ERROR_NONE, unlock);
				TDM_SNPRINTF(reply, len, "\t\t%s\t%d\t%d\n",
							 private_output->caps.props[i].name,
							 private_output->caps.props[i].id,
							 value.u32);
			}
		}
	}
	TDM_SNPRINTF(reply, len, "\n");

	/* layer information */
	TDM_SNPRINTF(reply, len, "[Layer information]\n");
	TDM_SNPRINTF(reply, len, "-----------------------------------------------------------------------\n");
	TDM_SNPRINTF(reply, len, "idx   output   zpos   buf   format   size   crop   geometry   transform\n");
	TDM_SNPRINTF(reply, len, "-----------------------------------------------------------------------\n");
	LIST_FOR_EACH_ENTRY(private_output, &private_display->output_list, link) {
		LIST_FOR_EACH_ENTRY(private_layer, &private_output->layer_list, link) {
			if (!private_layer->usable) {
				tdm_info_layer info;
				unsigned int format;
				tdm_size size;
				tbm_surface_info_s buf_info;

				TDM_DBG_GOTO_IF_FAIL(func_layer->layer_get_info, unlock);
				memset(&info, 0, sizeof info);
				ret = func_layer->layer_get_info(private_layer->layer_backend, &info);
				TDM_DBG_GOTO_IF_FAIL(ret == TDM_ERROR_NONE, unlock);

				format = tbm_surface_get_format(private_layer->showing_buffer);
				tbm_surface_get_info(private_layer->showing_buffer, &buf_info);

				if (IS_RGB(format))
					size.h = buf_info.planes[0].stride >> 2;
				else
					size.h = buf_info.planes[0].stride;
				size.v = tbm_surface_get_height(private_layer->showing_buffer);

				if (info.src_config.format)
					format = (info.src_config.format)?:format;

				TDM_SNPRINTF(reply, len, "%d   %d   %d   %p   %c%c%c%c   %dx%d   %dx%d+%d+%d   %dx%d+%d+%d   %s\n",
							 private_layer->index,
							 private_output->index,
							 private_layer->caps.zpos,
							 private_layer->showing_buffer, FOURCC_STR(format), size.h, size.v,
							 info.src_config.pos.w, info.src_config.pos.h, info.src_config.pos.x, info.src_config.pos.y,
							 info.dst_pos.w, info.dst_pos.h, info.dst_pos.x, info.dst_pos.y,
							 tdm_transform_str(info.transform));
			} else {
				TDM_SNPRINTF(reply, len, "%d   %d   %d   -\n",
							 private_layer->index,
							 private_output->index,
							 private_layer->caps.zpos);
			}

			TDM_SNPRINTF(reply, len, "\tcaps\t: ");
			tdm_layer_caps_str(private_layer->caps.capabilities, &reply, len);
			TDM_SNPRINTF(reply, len, "\n");

			TDM_SNPRINTF(reply, len, "\tformats\t: ");
			if (private_layer->caps.format_count > 0) {
				const char *sep = "";
				for (i = 0; i < private_layer->caps.format_count; i++) {
					TDM_SNPRINTF(reply, len, "%s%c%c%c%c", sep, FOURCC_STR(private_layer->caps.formats[i]));
					sep = ",";
				}
				TDM_SNPRINTF(reply, len, "\n");
			}

			TDM_SNPRINTF(reply, len, "\t%d properties:\n", private_layer->caps.prop_count);
			if (private_layer->caps.prop_count > 0) {
				TDM_SNPRINTF(reply, len, "\t\tname\tidx\tvalue\n");
				for (i = 0; i < private_layer->caps.prop_count; i++) {
					tdm_value value;
					TDM_DBG_GOTO_IF_FAIL(func_layer->layer_get_property, unlock);
					ret = func_layer->layer_get_property(private_layer->layer_backend,
														 private_layer->caps.props[i].id,
														 &value);
					TDM_DBG_GOTO_IF_FAIL(ret == TDM_ERROR_NONE, unlock);
					TDM_SNPRINTF(reply, len, "\t\t%s\t%d\t%d\n",
								 private_layer->caps.props[i].name,
								 private_layer->caps.props[i].id,
								 value.u32);
				}
			}
		}
	}
	TDM_SNPRINTF(reply, len, "\n");

	if (private_display->capabilities & TDM_DISPLAY_CAPABILITY_PP) {
		const char *sep = "";
		TDM_SNPRINTF(reply, len, "[PP information]\n");
		TDM_SNPRINTF(reply, len, "caps\t: ");
		tdm_pp_caps_str(private_display->caps_pp.capabilities, &reply, len);
		TDM_SNPRINTF(reply, len, "\n");
		TDM_SNPRINTF(reply, len, "formats\t: ");
		for (i = 0; i < private_display->caps_pp.format_count; i++) {
			TDM_SNPRINTF(reply, len, "%s%c%c%c%c", sep, FOURCC_STR(private_display->caps_pp.formats[i]));
			sep = ",";
		}
		TDM_SNPRINTF(reply, len, "\n");
		TDM_SNPRINTF(reply, len, "size\t: min(%dx%d) max(%dx%d) align_w(%d)\n",
					 private_display->caps_pp.min_w, private_display->caps_pp.min_h,
					 private_display->caps_pp.max_w, private_display->caps_pp.max_h,
					 private_display->caps_pp.preferred_align);
		if (!LIST_IS_EMPTY(&private_display->pp_list)) {
			TDM_SNPRINTF(reply, len, "-------------------------------------------------------------\n");
			TDM_SNPRINTF(reply, len, "src(format size crop)  |  dst(format size crop)  |  transform\n");
			TDM_SNPRINTF(reply, len, "-------------------------------------------------------------\n");
			LIST_FOR_EACH_ENTRY(private_pp, &private_display->pp_list, link) {
				TDM_SNPRINTF(reply, len, "%c%c%c%c %dx%d %dx%d+%d+%d | %c%c%c%c %dx%d %dx%d+%d+%d | %s\n",
							 FOURCC_STR(private_pp->info.src_config.format),
							 private_pp->info.src_config.size.h,
							 private_pp->info.src_config.size.v,
							 private_pp->info.src_config.pos.x, private_pp->info.src_config.pos.y,
							 private_pp->info.src_config.pos.w, private_pp->info.src_config.pos.h,
							 FOURCC_STR(private_pp->info.dst_config.format),
							 private_pp->info.dst_config.size.h,
							 private_pp->info.dst_config.size.v,
							 private_pp->info.dst_config.pos.x, private_pp->info.dst_config.pos.y,
							 private_pp->info.dst_config.pos.w, private_pp->info.dst_config.pos.h,
							 tdm_transform_str(private_pp->info.transform));
			}
		}
	} else {
		TDM_SNPRINTF(reply, len, "[No PP capability]\n");
	}
	TDM_SNPRINTF(reply, len, "\n");

	if (private_display->capabilities & TDM_DISPLAY_CAPABILITY_CAPTURE) {
		const char *sep = "";
		TDM_SNPRINTF(reply, len, "[Capture information]\n");
		TDM_SNPRINTF(reply, len, "caps\t: ");
		tdm_capture_caps_str(private_display->caps_capture.capabilities, &reply, len);
		TDM_SNPRINTF(reply, len, "\n");
		TDM_SNPRINTF(reply, len, "formats\t: ");
		for (i = 0; i < private_display->caps_capture.format_count; i++) {
			TDM_SNPRINTF(reply, len, "%s%c%c%c%c", sep, FOURCC_STR(private_display->caps_capture.formats[i]));
			sep = ",";
		}
		TDM_SNPRINTF(reply, len, "\n");
		TDM_SNPRINTF(reply, len, "size\t: min(%dx%d) max(%dx%d) align_w(%d)\n",
					 private_display->caps_capture.min_w, private_display->caps_capture.min_h,
					 private_display->caps_capture.max_w, private_display->caps_capture.max_h,
					 private_display->caps_capture.preferred_align);
		if (!LIST_IS_EMPTY(&private_display->capture_list)) {
			TDM_SNPRINTF(reply, len, "-----------------------------------\n");
			TDM_SNPRINTF(reply, len, "dst(format size crop)  |  transform\n");
			TDM_SNPRINTF(reply, len, "-----------------------------------\n");
			LIST_FOR_EACH_ENTRY(private_capture, &private_display->capture_list, link) {
				TDM_SNPRINTF(reply, len, "%c%c%c%c %dx%d %dx%d+%d+%d | %s\n",
							 FOURCC_STR(private_capture->info.dst_config.format),
							 private_capture->info.dst_config.size.h,
							 private_capture->info.dst_config.size.v,
							 private_capture->info.dst_config.pos.x, private_capture->info.dst_config.pos.y,
							 private_capture->info.dst_config.pos.w, private_capture->info.dst_config.pos.h,
							 tdm_transform_str(private_capture->info.transform));
			}
		}
	} else {
		TDM_SNPRINTF(reply, len, "[No Capture capability]\n");
	}
	TDM_SNPRINTF(reply, len, "\n");

unlock:
	_pthread_mutex_unlock(&private_display->lock);
}
