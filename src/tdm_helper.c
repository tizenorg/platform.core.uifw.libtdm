#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <png.h>
#include <string.h>
#include <tbm_surface.h>
#include <tbm_surface_internal.h>
#include <string.h>

#include "tdm.h"
#include "tdm_private.h"

#define PNG_DEPTH 8

static const char *dump_prefix[2] = {"png", "yuv"};

static void
_tdm_helper_dump_raw(const char *file, void *data1, int size1, void *data2,
                     int size2, void *data3, int size3)
{
    unsigned int *blocks;
    FILE *fp = fopen(file, "w+");
    TDM_RETURN_IF_FAIL(fp != NULL);

    blocks = (unsigned int*)data1;
    fwrite(blocks, 1, size1, fp);

    if (size2 > 0)
    {
        blocks = (unsigned int*)data2;
        fwrite(blocks, 1, size2, fp);
    }

    if (size3 > 0)
    {
        blocks = (unsigned int*)data3;
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
    if (!pPngStruct)
    {
        fclose(fp);
        return;
    }

    png_infop pPngInfo = png_create_info_struct(pPngStruct);
    if (!pPngInfo)
    {
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
        png_malloc(pPngStruct, height * sizeof(png_byte*));

    unsigned int *blocks = (unsigned int*)data;
    int y = 0;
    int x = 0;

    for (; y < height; ++y)
    {
        png_bytep row =
            png_malloc(pPngStruct, sizeof(png_byte) * width * pixel_size);
        row_pointers[y] = (png_bytep)row;
        for (x = 0; x < width; ++x)
        {
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

EXTERN void
tdm_helper_dump_buffer(tbm_surface_h buffer, const char *file)
{
    tbm_surface_info_s info;
    int len;
    const char *prefix;

    TDM_RETURN_IF_FAIL(buffer != NULL);
    TDM_RETURN_IF_FAIL(file != NULL);

    tbm_surface_get_info(buffer, &info);

    len = strnlen(file, 1024);
    if (info.format == TBM_FORMAT_ARGB8888 || info.format == TBM_FORMAT_XRGB8888)
        prefix = dump_prefix[0];
    else
        prefix = dump_prefix[1];

    if (strncmp(file + (len - 3), prefix, 3))
    {
        TDM_ERR("can't dump to '%s' file", file + (len - 3));
        return;
    }

    switch(info.format)
    {
    case TBM_FORMAT_ARGB8888:
    case TBM_FORMAT_XRGB8888:
        _tdm_helper_dump_png(file, info.planes[0].ptr,
                             info.planes[0].stride >> 2, info.height);
        break;
    case TBM_FORMAT_YVU420:
    case TBM_FORMAT_YUV420:
        _tdm_helper_dump_raw(file,
                             info.planes[0].ptr + info.planes[0].offset,
                             info.planes[0].stride * info.height,
                             info.planes[1].ptr + info.planes[1].offset,
                             info.planes[1].stride * (info.height >> 1),
                             info.planes[2].ptr + info.planes[2].offset,
                             info.planes[2].stride * (info.height >> 1));
        break;
    case TBM_FORMAT_NV12:
    case TBM_FORMAT_NV21:
        _tdm_helper_dump_raw(file,
                             info.planes[0].ptr + info.planes[0].offset,
                             info.planes[0].stride * info.height,
                             info.planes[1].ptr + info.planes[1].offset,
                             info.planes[1].stride * (info.height >> 1), NULL,
                             0);
        break;
    case TBM_FORMAT_YUYV:
    case TBM_FORMAT_UYVY:
        _tdm_helper_dump_raw(file,
                             info.planes[0].ptr + info.planes[0].offset,
                             info.planes[0].stride * info.height, NULL, 0,
                             NULL, 0);
        break;
    default:
        TDM_ERR("can't dump %c%c%c%c buffer", FOURCC_STR (info.format));
        return;
    }

    TDM_INFO("dump %s", file);
}
