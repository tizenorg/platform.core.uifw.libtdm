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
#include <poll.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>

#include "tdm_macro.h"

#include <tdm_client.h>
#include <tdm_helper.h>

int tdm_debug;

typedef struct _tdm_test_client_arg {
	char output_name[512];
	int fps;
	int sync;
	int interval;
	int offset;
	int enable_fake;
} tdm_test_client_arg;

typedef struct _tdm_test_client {
	tdm_test_client_arg args;

	int do_query;
	int do_vblank;
	int waiting;

	tdm_client *client;
} tdm_test_client;

struct typestrings {
	int type;
	char string[512];
};

struct optstrings {
	int  type;
	char opt[512];
	char desc[512];
	char arg[512];
	char ex[512];
};

enum {
	OPT_QRY,
	OPT_TST,
	OPT_GNR,
};

static struct typestrings typestrs[] = {
	{OPT_QRY, "Query"},
	{OPT_TST, "Test"},
	{OPT_GNR, "General"},
};

static struct optstrings optstrs[] = {
	{OPT_QRY, "qo", "output objects info", "<output_name>", "primary"},
	{OPT_TST, "v", "vblank test", "<output_name>[,<sync>][@<fps>][#<interval>][+<offset>][*fake]", "primary,0@60#1+0*1"},
};

#define DELIM "!@#^&*+-|,"

static char*
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

static void
usage(char *app_name)
{
	int type_size = sizeof(typestrs) / sizeof(struct typestrings);
	int opt_size = sizeof(optstrs) / sizeof(struct optstrings);
	int t;

	printf("usage: %s \n\n", app_name);

	for (t = 0; t < type_size; t++) {
		int o, f = 1;

		for (o = 0; o < opt_size; o++)
			if (optstrs[o].type == typestrs[t].type) {
				if (f == 1)
					printf(" %s options:\n\n", typestrs[t].string);
				printf("\t-%s\t%s\n", optstrs[o].opt, optstrs[o].desc);
				printf("\t\t%s\n", optstrs[o].arg);
				printf("\t\tex) %s\n", optstrs[o].ex);
				f = 0;
			}
	}

	exit(0);
}

//"<output_name>"
static void
parse_arg_qo(tdm_test_client *data, char *p)
{
	strtostr(data->args.output_name, 512, p, DELIM);
}

//"<output_name>[,<sync>][@<fps>][#<interval>][+<offset>][*fake]"
static void
parse_arg_v(tdm_test_client *data, char *p)
{
	char *end = p;

	end = strtostr(data->args.output_name, 512, p, DELIM);

	if (*end == ',') {
		p = end + 1;
		data->args.sync = strtol(p, &end, 10);
	}

	if (*end == '@') {
		p = end + 1;
		data->args.fps = strtol(p, &end, 10);
	}

	if (*end == '#') {
		p = end + 1;
		data->args.interval = strtol(p, &end, 10);
	}

	if (*end == '+' || *end == '-') {
		p = end;
		data->args.offset = strtol(p, &end, 10);
	}

	if (*end == '*') {
		p = end + 1;
		data->args.enable_fake= strtol(p, &end, 10);
	}
}

static void
parse_args(tdm_test_client *data, int argc, char *argv[])
{
	int size = sizeof(optstrs) / sizeof(struct optstrings);
	int i, j = 0;

	if (argc < 2) {
		usage(argv[0]);
		exit(1);
	}

	memset(data, 0, sizeof *data);
	data->args.interval = 1;

	for (i = 1; i < argc; i++) {
		for (j = 0; j < size; j++) {
			if (!strncmp(argv[i]+1, "qo", 512)) {
				data->do_query = 1;
				parse_arg_qo(data, argv[++i]);
				break;
			} else if (!strncmp(argv[i]+1, "v", 512)) {
				data->do_vblank = 1;
				parse_arg_v(data, argv[++i]);
				break;
			} else {
				usage(argv[0]);
				exit(1);
			}
		}
	}
}

static unsigned long
get_time_in_micros(void)
{
	struct timespec tp;

	if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
		return (unsigned long)(tp.tv_sec * 1000000) + (tp.tv_nsec / 1000L);

	return 0;
}

static void
_client_vblank_handler(tdm_client_vblank *vblank, tdm_error error, unsigned int sequence,
					   unsigned int tv_sec, unsigned int tv_usec, void *user_data)
{
	tdm_test_client *data = user_data;
	unsigned long cur, vbl;
	static unsigned long p_vbl = 0;

	data->waiting = 0;

	if (error == TDM_ERROR_DPMS_OFF) {
		printf("exit: dpms off\n");
		exit(1);
	}

	if (error != TDM_ERROR_NONE) {
		printf("exit: error(%d)\n", error);
		exit(1);
	}

	cur = get_time_in_micros();
	vbl = (unsigned long)tv_sec * (unsigned long)1000000 + (unsigned long)tv_usec;

	printf("vblank              : %ld us vbl(%lu)\n", vbl - p_vbl, vbl);

	if (cur - vbl > 2000) /* 2ms */
		printf("kernel -> tdm-client: %ld us\n", cur - vbl);

	if (tdm_debug) {
		static unsigned long p_cur = 0;
		printf("vblank event interval: %ld %ld\n",
			   vbl - p_vbl, cur - p_cur);
		p_cur = cur;
	}

	p_vbl = vbl;
}

static char *conn_str[3] = {"disconnected", "connected", "mode_setted"};
static char *dpms_str[4] = {"on", "standy", "suspend", "off"};

static void
_client_output_handler(tdm_client_output *output, tdm_output_change_type type,
					   tdm_value value, void *user_data)
{
	if (type == TDM_OUTPUT_CHANGE_CONNECTION)
		printf("output %s.\n", conn_str[value.u32]);
	else if (type == TDM_OUTPUT_CHANGE_DPMS)
		printf("dpms %s.\n", dpms_str[value.u32]);
}

static void
do_query(tdm_test_client *data)
{
	tdm_client_output *output;
	tdm_output_conn_status status;
	tdm_output_dpms dpms;
	unsigned int refresh;
	tdm_error error;

	output = tdm_client_get_output(data->client, NULL, &error);
	if (error != TDM_ERROR_NONE) {
		printf("tdm_client_get_output failed\n");
		return;
	}

	tdm_client_output_get_conn_status(output, &status);
	tdm_client_output_get_dpms(output, &dpms);
	tdm_client_output_get_refresh_rate(output, &refresh);

	printf("tdm_output \"%s\"\n", data->args.output_name);
	printf("\tstatus : %s\n", conn_str[status]);
	printf("\tdpms : %s\n", dpms_str[dpms]);
	printf("\trefresh : %d\n", refresh);
}

static void
do_vblank(tdm_test_client *data)
{
	tdm_client_output *output;
	tdm_client_vblank *vblank = NULL;
	tdm_error error;
	int fd = -1;
	struct pollfd fds;

	output = tdm_client_get_output(data->client, data->args.output_name, &error);
	if (error != TDM_ERROR_NONE) {
		printf("tdm_client_get_output failed\n");
		return;
	}

	tdm_client_output_add_change_handler(output, _client_output_handler, NULL);

	vblank = tdm_client_output_create_vblank(output, &error);
	if (error != TDM_ERROR_NONE) {
		printf("tdm_client_output_create_vblank failed\n");
		return;
	}

	tdm_client_vblank_set_enable_fake(vblank, data->args.enable_fake);
	tdm_client_vblank_set_sync(vblank, data->args.sync);
	if (data->args.fps > 0)
		tdm_client_vblank_set_fps(vblank, data->args.fps);
	tdm_client_vblank_set_offset(vblank, data->args.offset);

	error = tdm_client_get_fd(data->client, &fd);
	if (error != TDM_ERROR_NONE || fd < 0) {
		printf("tdm_client_get_fd failed\n");
		goto done;
	}

	fds.events = POLLIN;
	fds.fd = fd;
	fds.revents = 0;

	while (1) {
		int ret;

		if (!data->waiting) {
			error = tdm_client_vblank_wait(vblank, data->args.interval,
										   _client_vblank_handler, data);
			if (error == TDM_ERROR_DPMS_OFF) {
				printf("tdm_client_vblank_wait failed (dpms off)\n");
				goto done;
			}
			if (error != TDM_ERROR_NONE) {
				printf("tdm_client_vblank_wait failed (error: %d)\n", error);
				goto done;
			}
			data->waiting = 1;
		}

		if (!data->args.sync) {
			ret = poll(&fds, 1, -1);
			if (ret < 0) {
				if (errno == EINTR || errno == EAGAIN)  /* normal case */
					continue;
				else {
					printf("poll failed: %m\n");
					goto done;
				}
			}

			error = tdm_client_handle_events(data->client);
			if (error != TDM_ERROR_NONE) {
				printf("tdm_client_handle_events failed\n");
				goto done;
			}
		}
	}

done:
	if (vblank)
		tdm_client_vblank_destroy(vblank);
}

static tdm_test_client ttc_data;

int
main(int argc, char *argv[])
{
	tdm_test_client *data = &ttc_data;
	tdm_error error;
	const char *debug;

	debug = getenv("TDM_DEBUG");
	if (debug && (strstr(debug, "1")))
		tdm_debug = 1;

	parse_args(data, argc, argv);

	printf("sync(%d) fps(%d) interval(%d) offset(%d) enable_fake(%d)\n",
		   data->args.sync, data->args.fps, data->args.interval,
		   data->args.offset, data->args.enable_fake);

	data->client = tdm_client_create(&error);
	if (error != TDM_ERROR_NONE) {
		printf("tdm_client_create failed\n");
		goto done;
	}

	if (data->do_query)
		do_query(data);
	if (data->do_vblank)
		do_vblank(data);

done:
	if (data->client)
		tdm_client_destroy(data->client);

	return 0;
}
