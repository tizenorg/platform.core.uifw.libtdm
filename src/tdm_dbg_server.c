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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "tdm.h"
#include "tdm_private.h"
#include "tdm_helper.h"
#include "tdm_log.h"

#define TDM_DBG_SERVER_ARGS_MAX		32

static void _tdm_dbg_server_usage(char *app_name, char *reply, int *len);

static void
_tdm_dbg_server_query(unsigned int pid, char *cwd, int argc, char *argv[], char *reply, int *len, tdm_display *dpy)
{
	tdm_helper_get_display_information(dpy, reply, len);
}

static void
_tdm_dbg_server_dpms(unsigned int pid, char *cwd, int argc, char *argv[], char *reply, int *len, tdm_display *dpy)
{
	tdm_output *output;
	int output_idx, dpms_value;
	char *arg;
	char *end;
	tdm_error ret;

	if (argc < 3) {
		_tdm_dbg_server_usage(argv[0], reply, len);
		return;
	}

	arg = argv[2];
	output_idx = strtol(arg, &end, 10);
	if (*end != ':') {
		TDM_SNPRINTF(reply, len, "failed: no onoff value\n");
		return;
	}

	arg = end + 1;
	dpms_value = strtol(arg, &end, 10);

	output = tdm_display_get_output(dpy, output_idx, &ret);
	TDM_DBG_RETURN_IF_FAIL(ret == TDM_ERROR_NONE && output != NULL);

	ret = tdm_output_set_dpms(output, dpms_value);
	TDM_DBG_RETURN_IF_FAIL(ret == TDM_ERROR_NONE);

	TDM_SNPRINTF(reply, len, "done: DPMS %s\n", tdm_dpms_str(dpms_value));
}

static void
_tdm_dbg_server_debug(unsigned int pid, char *cwd, int argc, char *argv[], char *reply, int *len, tdm_display *dpy)
{
	int level;
	char *arg;
	char *end;

	if (argc < 3) {
		_tdm_dbg_server_usage(argv[0], reply, len);
		return;
	}

	arg = argv[2];
	level = strtol(arg, &end, 10);

	tdm_log_set_debug_level(level);
	TDM_SNPRINTF(reply, len, "debug level: %d\n", level);

	if (*end == '@') {
		char *arg = end + 1;

		tdm_display_enable_debug_module((const char *)arg);

		TDM_SNPRINTF(reply, len, "debugging... '%s'\n", arg);
	}
}

static void
_tdm_dbg_server_log_path(unsigned int pid, char *cwd, int argc, char *argv[], char *reply, int *len, tdm_display *dpy)
{
	static int old_stdout = -1;
	char fd_name[TDM_PATH_LEN];
	int  log_fd = -1;
	FILE *log_fl;
	char *path;

	if (argc < 3) {
		_tdm_dbg_server_usage(argv[0], reply, len);
		return;
	}

	if (old_stdout == -1)
		old_stdout = dup(STDOUT_FILENO);

	path = argv[2];
	TDM_DBG_RETURN_IF_FAIL(path != NULL);

	tdm_log_enable_dlog(0);

	if (!strncmp(path, "dlog", 4)) {
		tdm_log_enable_dlog(1);
		goto done;
	} else if (!strncmp(path, "console", 7))
		snprintf(fd_name, TDM_PATH_LEN, "/proc/%d/fd/1", pid);
	else {
		if (path[0] == '/')
			snprintf(fd_name, TDM_PATH_LEN, "%s", path);
		else {
			if (cwd)
				snprintf(fd_name, TDM_PATH_LEN, "%s/%s", cwd, path);
			else
				snprintf(fd_name, TDM_PATH_LEN, "%s", path);
		}
	}

	log_fl = fopen(fd_name, "a");
	if (!log_fl) {
		TDM_SNPRINTF(reply, len, "failed: open file(%s)\n", fd_name);
		return;
	}

	fflush(stderr);
	close(STDOUT_FILENO);

	setvbuf(log_fl, NULL, _IOLBF, 512);
	log_fd = fileno(log_fl);

	dup2(log_fd, STDOUT_FILENO);
	fclose(log_fl);
done:
	TDM_SNPRINTF(reply, len, "log path: '%s'\n", path);
}

static void
_tdm_dbg_server_prop(unsigned int pid, char *cwd, int argc, char *argv[], char *reply, int *len, tdm_display *dpy)
{
	tdm_output *output;
	tdm_output *layer = NULL;
	int output_idx, layer_idx = -1;
	int cnt, i, done = 0;
	tdm_value value;
	char temp[TDM_PATH_LEN];
	char *prop_name;
	char *arg;
	char *end;
	tdm_error ret;
	const tdm_prop *props;

	if (argc < 3) {
		_tdm_dbg_server_usage(argv[0], reply, len);
		return;
	}

	snprintf(temp, TDM_PATH_LEN, "%s", argv[2]);
	arg = temp;

	output_idx = strtol(arg, &end, 10);
	if (*end == ',') {
		arg = end + 1;
		layer_idx = strtol(arg, &end, 10);
	}

	if (*end != ':') {
		TDM_SNPRINTF(reply, len, "failed: no prop_name\n");
		return;
	}

	arg = end + 1;
	prop_name = strtok_r(arg, ",", &end);

	if (*end == '\0') {
		TDM_SNPRINTF(reply, len, "failed: no value\n");
		return;
	}

	arg = strtok_r(NULL, TDM_DELIM, &end);
	value.u32 = strtol(arg, &end, 10);

	output = tdm_display_get_output(dpy, output_idx, &ret);
	TDM_DBG_RETURN_IF_FAIL(ret == TDM_ERROR_NONE && output != NULL);

	if (layer_idx != -1) {
		layer = tdm_output_get_layer(output, layer_idx, &ret);
		TDM_DBG_RETURN_IF_FAIL(ret == TDM_ERROR_NONE && layer != NULL);
	}

	if (layer) {
		ret = tdm_layer_get_available_properties(layer, &props, &cnt);
		TDM_DBG_RETURN_IF_FAIL(ret == TDM_ERROR_NONE);

		for (i = 0; i < cnt; i++) {
			if (!strncmp(props[i].name, prop_name, TDM_NAME_LEN)) {
				ret = tdm_layer_set_property(layer, props[i].id, value);
				TDM_DBG_RETURN_IF_FAIL(ret == TDM_ERROR_NONE);
				done = 1;
				break;
			}
		}
	} else {
		ret = tdm_output_get_available_properties(output, &props, &cnt);
		TDM_DBG_RETURN_IF_FAIL(ret == TDM_ERROR_NONE);

		for (i = 0; i < cnt; i++) {
			if (!strncmp(props[i].name, prop_name, TDM_NAME_LEN)) {
				ret = tdm_output_set_property(output, props[i].id, value);
				TDM_DBG_RETURN_IF_FAIL(ret == TDM_ERROR_NONE);
				done = 1;
				break;
			}
		}
	}

	if (done)
		TDM_SNPRINTF(reply, len, "done: %s:%d \n", prop_name, value.u32);
	else
		TDM_SNPRINTF(reply, len, "no '%s' propperty \n", prop_name);
}

static void
_tdm_dbg_server_dump(unsigned int pid, char *cwd, int argc, char *argv[], char *reply, int *len, tdm_display *dpy)
{
	if (argc < 3) {
		_tdm_dbg_server_usage(argv[0], reply, len);
		return;
	}

	tdm_display_enable_dump((const char*)argv[2]);

	TDM_SNPRINTF(reply, len, "%s done\n", argv[2]);
}

static struct {
	const char *opt;
	void (*func)(unsigned int pid, char *cwd, int argc, char *argv[], char *reply, int *len, tdm_display *dpy);
	const char *desc;
	const char *arg;
	const char *ex;
} option_proc[] = {
	{
		"info", _tdm_dbg_server_query,
		"show tdm output, layer information", NULL, NULL
	},
	{
		"dpms", _tdm_dbg_server_dpms,
		"set output dpms", "<output_idx>:<dpms>", "0:3 or 0:0"
	},
	{
		"debug", _tdm_dbg_server_debug,
		"set the debug level and modules(none,mutex,buffer,thread)",
		"<level>[@<module1>[,<module2>]]",
		NULL
	},
	{
		"log_path", _tdm_dbg_server_log_path,
		"set the log path (console,dlog,filepath)",
		"<path>",
		"console"
	},
	{
		"prop", _tdm_dbg_server_prop,
		"set the property of a output or a layer",
		"<output_idx>[,<layer_idx>]:<prop_name>,<value>",
		NULL
	},
	{
		"dump", _tdm_dbg_server_dump,
		"dump buffers (type: layer, pp, capture, none)",
		"<object_type1>[,<object_type2>[,...]]",
		NULL
	},
};

static void
_tdm_dbg_server_usage(char *app_name, char *reply, int *len)
{
	int opt_size = sizeof(option_proc) / sizeof(option_proc[0]);
	int i;

	TDM_SNPRINTF(reply, len, "usage: %s \n\n", app_name);

	for (i = 0; i < opt_size; i++) {
		TDM_SNPRINTF(reply, len, "\t-%s\t%s\n", option_proc[i].opt, option_proc[i].desc);
		if (option_proc[i].arg)
			TDM_SNPRINTF(reply, len, "\t\t  %s\n", option_proc[i].arg);
		if (option_proc[i].ex)
			TDM_SNPRINTF(reply, len, "\t\t  ex) %s\n", option_proc[i].ex);
		TDM_SNPRINTF(reply, len, "\n");
	}
}

static void
_tdm_dbg_server_command(unsigned int pid, char *cwd, tdm_display *dpy, int argc, char *argv[], char *reply, int *len)
{
	int opt_size = sizeof(option_proc) / sizeof(option_proc[0]);
	int i;

	if (argc < 2) {
		_tdm_dbg_server_usage(argv[0], reply, len);
		return;
	}

	for (i = 0; i < opt_size; i++) {
		if (argv[1][0] == '-' && !strncmp(argv[1] + 1, option_proc[i].opt, 32)) {
			if (option_proc[i].func) {
				option_proc[i].func(pid, cwd, argc, argv, reply, len, dpy);
				return;
			} else {
				TDM_SNPRINTF(reply, len, "'%s' not implemented.\n", argv[1]);
				return;
			}
		}
	}

	_tdm_dbg_server_usage(argv[0], reply, len);
	return;
}


INTERN void
tdm_dbg_server_command(tdm_display *dpy, const char *options, char *reply, int *len)
{
	unsigned int pid;
	char cwd[1024];
	int argc = 0;
	char *argv[TDM_DBG_SERVER_ARGS_MAX] = {0,};
	char temp[1024];
	char *arg;
	char *end = NULL, *e;

	snprintf(temp, sizeof(temp), "%s", options);

	arg = strtok_r(temp, " ", &end);
	if (!arg) {
		TDM_SNPRINTF(reply, len, "no pid for tdm-dbg");
		return;
	}
	pid = strtol(arg, &e, 10);

	arg = strtok_r(NULL, " ", &end);
	if (!arg) {
		TDM_SNPRINTF(reply, len, "no cwd for tdm-dbg");
		return;
	}
	snprintf(cwd, sizeof(cwd), "%s", arg);

	TDM_DBG("pid(%d) cwd(%s)", pid, cwd);

	argv[argc] = strtok_r(NULL, " ", &end);
	while (argv[argc]) {
		argc++;
		if (argc == TDM_DBG_SERVER_ARGS_MAX) {
			TDM_SNPRINTF(reply, len, "too many arguments for tdm-dbg");
			break;
		}
		argv[argc] = strtok_r(NULL, " ", &end);
	}

	_tdm_dbg_server_command(pid, cwd, dpy, argc, argv, reply, len);
}
