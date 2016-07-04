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

#include <tdm_log.h>
#include <tdm_macro.h>
#include <tdm_list.h>
#include <tdm-client-protocol.h>

#undef exit_if_fail
#define exit_if_fail(cond) { \
	if (!(cond)) { \
		printf("'%s' failed. (line:%d)\n", #cond, __LINE__); \
		exit(0); \
	} \
}

typedef struct _tdm_dbg_info {
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_tdm *tdm;
} tdm_dbg_info;

static tdm_dbg_info td_info;
static int done;

static void
_tdm_dbg_cb_debug_done(void *data, struct wl_tdm *wl_tdm, const char *message)
{
	printf("%s", message);

	done = 1;
}

static const struct wl_tdm_listener  tdm_dbg_listener = {
	_tdm_dbg_cb_debug_done,
};

static void
_tdm_dbg_cb_global(void *data, struct wl_registry *registry,
				   uint32_t name, const char *interface,
				   uint32_t version)
{
	tdm_dbg_info *info = data;

	if (strncmp(interface, "wl_tdm", 6) == 0) {
		info->tdm = wl_registry_bind(registry, name, &wl_tdm_interface, version);
		exit_if_fail(info->tdm != NULL);
		wl_tdm_add_listener(info->tdm, &tdm_dbg_listener, info);
		wl_display_flush(info->display);
	}
}

static void
_tdm_dbg_cb_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
}

static const struct wl_registry_listener tdm_dbg_registry_listener = {
	_tdm_dbg_cb_global,
	_tdm_dbg_cb_global_remove
};

int
main(int argc, char ** argv)
{
	tdm_dbg_info *info = &td_info;
	int i, ret = 0;
	char cwd[1024];
	char options[1024];
	int bufsize = sizeof(options);
	char *str_buf = options;
	int *len_buf = &bufsize;
	const char *xdg;

	xdg = (const char*)getenv("XDG_RUNTIME_DIR");
	if (!xdg) {
		char buf[32];
		snprintf(buf, sizeof(buf), "/run");

		ret = setenv("XDG_RUNTIME_DIR", (const char*)buf, 1);
		exit_if_fail(ret == 0);
	}

	info->display = wl_display_connect("tdm-socket");
	exit_if_fail(info->display != NULL);

	info->registry = wl_display_get_registry(info->display);
	exit_if_fail(info->registry != NULL);

	wl_registry_add_listener(info->registry,
							 &tdm_dbg_registry_listener, info);
	wl_display_roundtrip(info->display);
	exit_if_fail(info->tdm != NULL);

	TDM_SNPRINTF(str_buf, len_buf, "%d ", getpid());

	if (!getcwd(cwd, sizeof(cwd)))
		snprintf(cwd, sizeof(cwd), "/tmp");
	TDM_SNPRINTF(str_buf, len_buf, "%s ", cwd);

	for (i = 0; i < argc; i++)
		TDM_SNPRINTF(str_buf, len_buf, "%s ", argv[i]);

	done = 0;

	wl_tdm_debug(info->tdm, options);

	while (!done && ret >= 0)
		ret = wl_display_dispatch(info->display);

	if (info->tdm)
		wl_tdm_destroy(info->tdm);
	if (info->registry)
		wl_registry_destroy(info->registry);
	if (info->display)
		wl_display_disconnect(info->display);

	return 0;
}
