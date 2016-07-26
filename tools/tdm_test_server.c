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
#include <signal.h>
#include <poll.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>

#include <tbm_surface.h>
#include <tbm_surface_internal.h>

#include <tdm.h>
#include <tdm_log.h>
#include <tdm_list.h>
#include <tdm_helper.h>
#include <tdm_backend.h>

#include "tdm_macro.h"
#include "tdm_private.h"
#include "buffers.h"

////////////////////////////////////////////////////////////////////////////////
struct typestrings {
	int type;
	const char *string;
	const char *desc;
};

struct optstrings {
	int  type;
	const char *opt;
	const char *desc;
	const char *arg;
	const char *ex;
};

enum {
	OPT_QRY,
	OPT_TST,
	OPT_GEN,
};

static struct typestrings typestrs[] = {
	{OPT_QRY, "Query",          NULL},
	{OPT_TST, "Test",           NULL},
	{OPT_GEN, "General",        NULL},
};

static struct optstrings optstrs[] = {
	{
		OPT_QRY, "q", "show tdm output, layer information",
		NULL, NULL
	},
	{
		OPT_TST, "a", "set all layer objects for all connected outputs",
		NULL, NULL
	},
	{
		OPT_TST, "o", "set a mode for a output object",
		"<output_idx>@<mode>[&<refresh>]", "0@1920x1080"
	},
	{
		OPT_TST, "l", "set a layer object",
		"<layer_idx>[:<w>x<h>[+<x>+<y>][,<h>x<v>][@<format>]]~<w>x<h>[+<x>+<y>][*<transform>]", NULL
	},
	{
		OPT_TST, "p", "set a PP object.\n\t\t'-l' is used to show the result on screen.",
		"<w>x<h>[+<x>+<y>][,<h>x<v>][@<format>]~<w>x<h>[+<x>+<y>][,<h>x<v>][@<format>][*<transform>][&<fps>]", NULL
	},
	{
		OPT_TST, "c", "catpure a output object or a layer object.\n\t\t'-l' is used to show the result on screen.",
		"<output_idx>[,<layer_idx>]~<w>x<h>[+<x>+<y>][,<h>x<v>][@<format>][*<transform>]", NULL
	},
	{
		OPT_GEN, "w", "set the property of a object",
		"<prop_name>:<value>", NULL
	},
	{
		OPT_GEN, "b", "set the fill(smtpe,tiles,plain) and framebuffer type(scanout,noncachable,wc)",
		"<fill>[:<buf_flag>[,<buf_flag2>[,...]]]", NULL
	},
	{
		OPT_GEN, "v", "update layers every vblank",
		NULL, NULL
	},
};

struct usagestring {
	const char *string;
	const char *desc;
};

static struct usagestring usages[] = {
	{
		"-q",
		NULL
	},
	{
		"-a -b plain",
		"test all outputs, layers with plain buffers"
	},
	{
		"-o 0@1920x1080",
		"Set the \"1920x1080\" mode to the output 0. And show a buffer via a primary layer of the output 0"
	},
	{
		"-o 0@1920x1080 -l 1~640x480+50+100",
		"Create the 640x480 buffer and show it in the (50,100) pos of screen via the layer 1"
	},
	{
		"-p 320x240@NV12~480x360+80+40,640x480@AR24 -l 1~640x480+50+100",
		"Convert the 320x240@NV12 buffer to the 640x480@AR24 buffer(480x360+80+40) and show the result via the layer 1"
	},
};

static void
usage(char *app_name)
{
	int type_size = sizeof(typestrs) / sizeof(struct typestrings);
	int opt_size = sizeof(optstrs) / sizeof(struct optstrings);
	int usages_size = sizeof(usages) / sizeof(struct usagestring);
	int t;

	printf("usage: %s \n\n", app_name);

	for (t = 0; t < type_size; t++) {
		int o, f = 1;

		for (o = 0; o < opt_size; o++)
			if (optstrs[o].type == typestrs[t].type) {
				if (f == 1)
					printf(" %s options: %s\n\n", typestrs[t].string, (typestrs[t].desc) ? : "");
				printf("\t-%s\t%s\n", optstrs[o].opt, optstrs[o].desc);
				if (optstrs[o].arg)
					printf("\t\t  %s\n", optstrs[o].arg);
				if (optstrs[o].ex)
					printf("\t\t  ex) %s\n", optstrs[o].ex);
				f = 0;
			}
		printf("\n");
	}

	printf(" For example)\n\n");

	for (t = 0; t < usages_size; t++) {
		printf("    $ %s %s\n", app_name, usages[t].string);
		printf("\t%s\n", usages[t].desc);
	}
	printf("\n");
	exit(0);
}

////////////////////////////////////////////////////////////////////////////////

static const char *tdm_buf_flag_names[] = {
	"scanout",
	"noncachable",
	"wc",
};
TDM_BIT_NAME_FB(buf_flag)

#define DEFAULT_FORMAT	TBM_FORMAT_ARGB8888

#define print_size(s) \
	printf("%dx%d", (s)->h, (s)->v)
#define print_pos(p) \
	printf("%dx%d+%d+%d", (p)->w, (p)->h, (p)->x, (p)->y)
#define print_format(f) \
	if (f) printf("%c%c%c%c", FOURCC_STR(f)); \
	else printf("NONE")
#define print_config(c) \
	do { \
		print_size(&(c)->size); \
		printf(" "); \
		print_pos(&(c)->pos); \
		printf(" "); \
		print_format((c)->format); \
	} while (0)
#define print_prop(w) \
	printf("%s(%d)", (w)->name, ((w)->value).u32)

typedef struct _tdm_test_server tdm_test_server;
typedef struct _tdm_test_server_layer tdm_test_server_layer;
typedef struct _tdm_test_server_capture tdm_test_server_capture;

typedef struct _tdm_test_server_prop {
	/* args */
	char name[TDM_NAME_LEN];
	tdm_value value;

	/* variables for test */
	struct list_head link;
} tdm_test_server_prop;

typedef struct _tdm_test_server_buffer {
	/* variables for test */
	tbm_surface_h b;
	int in_use;
	tdm_test_server_layer *l;

	tdm_test_server_capture *c;
	tdm_buffer_release_handler done;
} tdm_test_server_buffer;

typedef struct _tdm_test_server_output {
	/* args */
	int idx;
	char mode[TDM_NAME_LEN];
	int refresh;

	/* variables for test */
	struct list_head link;
	struct list_head prop_list;
	struct list_head layer_list;
	tdm_test_server *data;
	tdm_output *output;

	int fill_primary_layer;
} tdm_test_server_output;

typedef struct _tdm_test_server_pp {
	/* args */
	tdm_info_pp info;
	int fps;

	/* variables for test */
	struct list_head link;
	tdm_test_server *data;
	tdm_test_server_layer *l;
	tdm_pp *pp;
	tbm_surface_h bufs[6];
	int buf_idx;

	tdm_event_loop_source *timer_source;
} tdm_test_server_pp;

struct _tdm_test_server_capture {
	/* args */
	int output_idx;
	int layer_idx;
	tdm_info_capture info;

	/* variables for test */
	struct list_head link;
	tdm_test_server *data;
	tdm_test_server_layer *l;
	tdm_capture *capture;
};

struct _tdm_test_server_layer {
	/* args */
	int idx;
	tdm_info_layer info;

	/* variables for test */
	struct list_head link;
	struct list_head prop_list;
	tdm_test_server *data;
	tdm_test_server_output *o;
	tdm_layer *layer;
	int is_primary;
	tdm_test_server_pp *owner_p;
	tdm_test_server_capture *owner_c;
	tbm_surface_h bufs[3];
	int buf_idx;
};

struct _tdm_test_server {
	/* args */
	int do_query;
	int do_all;
	int do_vblank;
	int bflags;
	int b_fill;

	/* variables for test */
	struct list_head output_list;
	struct list_head pp_list;
	struct list_head capture_list;
	tdm_display *display;
};

static void run_test(tdm_test_server *data);
static void output_setup(tdm_test_server_output *o);
static void layer_show_buffer(tdm_test_server_layer *l, tbm_surface_h b);
static void capture_attach(tdm_test_server_capture *c, tbm_surface_h b);

static char*
parse_size(tdm_size *size, char *arg)
{
	char *end;
	size->h = strtol(arg, &end, 10);
	TDM_EXIT_IF_FAIL(*end == 'x');
	arg = end + 1;
	size->v = strtol(arg, &end, 10);
	return end;
}

static char*
parse_pos(tdm_pos *pos, char *arg)
{
	char *end;
	pos->w = strtol(arg, &end, 10);
	TDM_EXIT_IF_FAIL(*end == 'x');
	arg = end + 1;
	pos->h = strtol(arg, &end, 10);
	if (*end == '+') {
		arg = end + 1;
		pos->x = strtol(arg, &end, 10);
		TDM_EXIT_IF_FAIL(*end == '+');
		arg = end + 1;
		pos->y = strtol(arg, &end, 10);
	}
	return end;
}

static char*
parse_config(tdm_info_config *config, char *arg)
{
	char *end;
	end = parse_pos(&config->pos, arg);
	if (*end == ',') {
		arg = end + 1;
		end = parse_size(&config->size, arg);
	}
	if (*end == '@') {
		char temp[32];
		arg = end + 1;
		end = strtostr(temp, 32, arg, TDM_DELIM);
		config->format = FOURCC_ID(temp);
	}
	return end;
}

static void
parse_arg_o(tdm_test_server_output *o, char *arg)
{
	char *end;
	TDM_EXIT_IF_FAIL(arg != NULL);
	o->idx = strtol(arg, &end, 10);
	TDM_EXIT_IF_FAIL(*end == '@');
	arg = end + 1;
	end = strtostr(o->mode, TDM_NAME_LEN, arg, TDM_DELIM);
	if (*end == ',') {
		arg = end + 1;
		o->refresh = strtol(arg, &end, 10);
	}
}

static void
parse_arg_p(tdm_test_server_pp *p, char *arg)
{
	tdm_info_pp *pp_info = &p->info;
	char *end;
	TDM_EXIT_IF_FAIL(arg != NULL);
	end = parse_config(&pp_info->src_config, arg);
	TDM_EXIT_IF_FAIL(*end == '~');
	arg = end + 1;
	end = parse_config(&pp_info->dst_config, arg);
	if (*end == '*') {
		arg = end + 1;
		pp_info->transform = strtol(arg, &end, 10);
	}
	if (*end == '&') {
		arg = end + 1;
		p->fps = strtol(arg, &end, 10);
	}
}

static void
parse_arg_c(tdm_test_server_capture *c, char *arg)
{
	tdm_info_capture *capture_info = &c->info;
	char *end;
	TDM_EXIT_IF_FAIL(arg != NULL);
	c->output_idx = strtol(arg, &end, 10);
	if (*end == ',') {
		arg = end + 1;
		c->layer_idx = strtol(arg, &end, 10);
	}
	TDM_EXIT_IF_FAIL(*end == '~');
	arg = end + 1;
	end = parse_config(&capture_info->dst_config, arg);
	if (*end == '*') {
		arg = end + 1;
		capture_info->transform = strtol(arg, &end, 10);
	}
}

static void
parse_arg_l(tdm_test_server_layer *l, char *arg)
{
	tdm_info_layer *layer_info = &l->info;
	char *end;
	TDM_EXIT_IF_FAIL(arg != NULL);
	l->idx = strtol(arg, &end, 10);
	if (*end == ':') {
		arg = end + 1;
		end = parse_config(&layer_info->src_config, arg);
	}
	TDM_EXIT_IF_FAIL(*end == '~');
	arg = end + 1;
	end = parse_pos(&layer_info->dst_pos, arg);
	if (*end == '*') {
		arg = end + 1;
		layer_info->transform = strtol(arg, &end, 10);
	}
}

static void
parse_arg_w(tdm_test_server_prop *w, char *arg)
{
	char *end;
	TDM_EXIT_IF_FAIL(arg != NULL);
	end = strtostr(w->name, TDM_PATH_LEN, arg, TDM_DELIM);
	TDM_EXIT_IF_FAIL(*end == ':');
	arg = end + 1;
	w->value.u32 = strtol(arg, &end, 10);
}

static void
parse_arg_b(tdm_test_server *data, char *arg)
{
	char *end = arg;
	char temp[TDM_NAME_LEN] = {0,};
	TDM_EXIT_IF_FAIL(arg != NULL);

	end = strtostr(temp, 32, arg, TDM_DELIM);
	if (!strncmp(temp, "smpte", 5))
		data->b_fill = PATTERN_SMPTE;
	else if (!strncmp(temp, "tiles", 5))
		data->b_fill = PATTERN_TILES;
	else if (!strncmp(temp, "plain", 5))
		data->b_fill = PATTERN_PLAIN;
	else {
		printf("'%s': unknown flag\n", temp);
		exit(0);
	}

	if (*arg == ':') {
		data->bflags = 0;
		arg = end + 1;
		snprintf(temp, TDM_NAME_LEN, "%s", arg);
		arg = strtok_r(temp, ",", &end);
		while (arg) {
			if (!strncmp(arg, "default", 7))
				printf("Ignore '%s' flag\n", arg);
			else if (!strncmp(arg, "scanout", 7))
				data->bflags |= TBM_BO_SCANOUT;
			else if (!strncmp(arg, "noncachable", 11))
				data->bflags |= TBM_BO_NONCACHABLE;
			else if (!strncmp(arg, "wc", 2))
				data->bflags |= TBM_BO_WC;
			else {
				printf("'%s': unknown flag\n", arg);
				exit(0);
			}
			arg = strtok_r(NULL, ",", &end);
		}
	}
}

static void
parse_args(tdm_test_server *data, int argc, char *argv[])
{
	tdm_test_server_output *o = NULL;
	tdm_test_server_layer *l = NULL;
	tdm_test_server_pp *p = NULL;
	tdm_test_server_capture *c = NULL;
	tdm_test_server_prop *w = NULL;
	void *last_option = NULL;
	void *last_object = NULL;
	int i;

	if (argc < 2) {
		usage(argv[0]);
		exit(0);
	}

	for (i = 1; i < argc; i++) {
		if (!strncmp(argv[i] + 1, "q", 1)) {
			data->do_query = 1;
			return;
		} else if (!strncmp(argv[i] + 1, "a", 1)) {
			data->do_all = 1;
		} else if (!strncmp(argv[i] + 1, "o", 1)) {
			TDM_GOTO_IF_FAIL(data->do_all == 0, all);
			o = calloc(1, sizeof * o);
			TDM_EXIT_IF_FAIL(o != NULL);
			o->data = data;
			LIST_INITHEAD(&o->layer_list);
			LIST_INITHEAD(&o->prop_list);
			LIST_ADDTAIL(&o->link, &data->output_list);
			parse_arg_o(o, argv[++i]);
			last_object = o;
		} else if (!strncmp(argv[i] + 1, "p", 1)) {
			TDM_GOTO_IF_FAIL(data->do_all == 0, all);
			p = calloc(1, sizeof * p);
			TDM_EXIT_IF_FAIL(p != NULL);
			p->data = data;
			p->fps = 30;   /* default 30 fps */
			LIST_ADDTAIL(&p->link, &data->pp_list);
			parse_arg_p(p, argv[++i]);
			last_option = p;
		} else if (!strncmp(argv[i] + 1, "c", 1)) {
			TDM_GOTO_IF_FAIL(data->do_all == 0, all);
			c = calloc(1, sizeof * c);
			TDM_EXIT_IF_FAIL(c != NULL);
			c->data = data;
			c->output_idx = -1;
			c->layer_idx = -1;
			LIST_ADDTAIL(&c->link, &data->capture_list);
			parse_arg_c(c, argv[++i]);
			last_option = c;
		} else if (!strncmp(argv[i] + 1, "l", 1)) {
			TDM_GOTO_IF_FAIL(data->do_all == 0, all);
			if (!o) {
				o = calloc(1, sizeof * o);
				TDM_EXIT_IF_FAIL(o != NULL);
				o->data = data;
				LIST_INITHEAD(&o->layer_list);
				LIST_INITHEAD(&o->prop_list);
				LIST_ADDTAIL(&o->link, &data->output_list);
			}
			l = calloc(1, sizeof * l);
			TDM_EXIT_IF_FAIL(l != NULL);
			LIST_INITHEAD(&l->prop_list);
			LIST_ADDTAIL(&l->link, &o->layer_list);
			l->data = data;
			l->o = o;
			parse_arg_l(l, argv[++i]);
			if (p && last_option == p) {
				p->l = l;
				l->owner_p = p;
			} else if (c && last_option == c) {
				c->l = l;
				l->owner_c = c;
			}
			last_object = l;
		} else if (!strncmp(argv[i] + 1, "w", 1)) {
			TDM_GOTO_IF_FAIL(data->do_all == 0, all);
			if (!last_object)
				goto no_object;
			w = calloc(1, sizeof * w);
			TDM_EXIT_IF_FAIL(w != NULL);
			if (o && last_object == o)
				LIST_ADDTAIL(&w->link, &o->prop_list);
			else if (l && last_object == l)
				LIST_ADDTAIL(&w->link, &l->prop_list);
			parse_arg_w(w, argv[++i]);
		} else if (!strncmp(argv[i] + 1, "b", 1)) {
			parse_arg_b(data, argv[++i]);
		} else if (!strncmp(argv[i] + 1, "v", 1)) {
			data->do_vblank = 1;
		} else {
			usage(argv[0]);
			exit(0);
		}
	}

	LIST_FOR_EACH_ENTRY(p, &data->pp_list, link) {
		if (!p->l)
			goto no_layer;
	}
	LIST_FOR_EACH_ENTRY(c, &data->capture_list, link) {
		if (!c->l)
			goto no_layer;
	}

	return;
no_layer:
	printf("Use '-l' to set a layer for '-p' or '-c'.\n");
	exit(0);
no_object:
	printf("Use '-o' or '-l' to set a object first.\n");
	exit(0);
all:
	printf("Can't use '-%s' with '-a'.\n", argv[i] + 1);
	exit(0);
}

static void
interpret_args(tdm_test_server *data)
{
	tdm_test_server_output *o = NULL;
	tdm_test_server_layer *l = NULL;
	tdm_error ret;

	/* create the objects of outputs */
	if (data->do_all) {
		int i, output_count;

		ret = tdm_display_get_output_count(data->display, &output_count);
		TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

		for (i = 0; i < output_count; i++) {
			o = calloc(1, sizeof * o);
			TDM_EXIT_IF_FAIL(o != NULL);
			o->data = data;
			LIST_INITHEAD(&o->layer_list);
			LIST_INITHEAD(&o->prop_list);
			LIST_ADDTAIL(&o->link, &data->output_list);
			o->idx = i;
			o->fill_primary_layer = 1;
		}
	}

	/* check if the primary layer object exists */
	LIST_FOR_EACH_ENTRY(o, &data->output_list, link) {
		tdm_output *output;
		const tdm_output_mode *mode;
		int j, layer_count, primary_index = 0;
		tdm_test_server_layer *primary_l = NULL;

		output_setup(o);

		output = tdm_display_get_output(data->display, o->idx, &ret);
		TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);
		ret = tdm_output_get_mode(output, &mode);
		TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);
		ret = tdm_output_get_layer_count(output, &layer_count);
		TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);
		ret = tdm_output_get_primary_index(output, &primary_index);
		TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

		if (LIST_IS_EMPTY(&o->layer_list))
			o->fill_primary_layer = 1;
		else {
			LIST_FOR_EACH_ENTRY(l, &o->layer_list, link) {
				if (l->idx == primary_index) {
					l->is_primary = 1;
					o->fill_primary_layer = 1;
					primary_l = l;
					break;
				}
			}
		}

		if (!primary_l || data->do_all) {
			for (j = 0; j < layer_count; j++) {
				if (j != primary_index && !data->do_all)
					continue;
				l = calloc(1, sizeof * l);
				TDM_EXIT_IF_FAIL(l != NULL);
				LIST_INITHEAD(&l->prop_list);
				LIST_ADDTAIL(&l->link, &o->layer_list);
				l->data = data;
				l->o = o;
				l->idx = j;
				if (j == primary_index) {
					l->is_primary = 1;
					l->info.dst_pos.w = mode->hdisplay;
					l->info.dst_pos.h = mode->vdisplay;
					primary_l = l;
				} else {
					l->info.dst_pos.w = TDM_ALIGN(mode->hdisplay / 3, 2);
					l->info.dst_pos.h = TDM_ALIGN(mode->vdisplay / 3, 2);
					l->info.dst_pos.x = TDM_ALIGN(((mode->hdisplay / 3) / layer_count) * j, 2);
					l->info.dst_pos.y = TDM_ALIGN(((mode->vdisplay / 3) / layer_count) * j, 2);
				}
			}
		}

		TDM_EXIT_IF_FAIL(primary_l != NULL);
		LIST_DEL(&primary_l->link);
		LIST_ADD(&primary_l->link, &o->layer_list);
	}

	/* fill the empty information of layers */
	LIST_FOR_EACH_ENTRY(o, &data->output_list, link) {
		tdm_output *output;
		int minw, minh, maxw, maxh;

		output = tdm_display_get_output(data->display, o->idx, &ret);
		TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);
		ret = tdm_output_get_available_size(output, &minw, &minh, &maxw, &maxh, NULL);
		TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

		/* l->info.src_config.size will be decided when a buffer shows really */
		LIST_FOR_EACH_ENTRY(l, &o->layer_list, link) {
			if (minw > 0 && minh > 0) {
				TDM_EXIT_IF_FAIL(l->info.dst_pos.w >= minw);
				TDM_EXIT_IF_FAIL(l->info.dst_pos.h >= minh);
			}
			if (maxw > 0 && maxh > 0) {
				TDM_EXIT_IF_FAIL(l->info.dst_pos.w <= maxw);
				TDM_EXIT_IF_FAIL(l->info.dst_pos.h <= maxh);
			}

			if (l->owner_p) {
				l->info.src_config.format = l->owner_p->info.dst_config.format;
				if (l->info.src_config.pos.w == 0) {
					TDM_EXIT_IF_FAIL(l->owner_p->info.dst_config.size.h > 0);
					l->info.src_config.pos.w = l->owner_p->info.dst_config.size.h;
					l->info.src_config.pos.h = l->owner_p->info.dst_config.size.v;
				}
			} else if (l->owner_c) {
				l->info.src_config.format = l->owner_c->info.dst_config.format;
				if (l->info.src_config.pos.w == 0) {
					TDM_EXIT_IF_FAIL(l->owner_c->info.dst_config.size.h > 0);
					l->info.src_config.pos.w = l->owner_c->info.dst_config.size.h;
					l->info.src_config.pos.h = l->owner_c->info.dst_config.size.v;
				}
			} else {
				if (l->info.src_config.pos.w == 0) {
					TDM_EXIT_IF_FAIL(l->info.dst_pos.w > 0);
					l->info.src_config.pos.w = l->info.dst_pos.w;
					l->info.src_config.pos.h = l->info.dst_pos.h;
				}
			}
		}
	}
}

static tdm_test_server tts_data;
static int tts_buffer_key;
#define TTS_BUFFER_KEY ((unsigned long)&tts_buffer_key)

static tbm_surface_h
buffer_allocate(int width, int height, int format, int flags)
{
	tdm_test_server_buffer *tb = calloc(1, sizeof *tb);
	TDM_EXIT_IF_FAIL(tb != NULL);
	tb->b = tbm_surface_internal_create_with_flags(width, height, format, flags);
	TDM_EXIT_IF_FAIL(tb->b != NULL);
	tdm_helper_clear_buffer(tb->b);
	tbm_surface_internal_add_user_data(tb->b, TTS_BUFFER_KEY, free);
	tbm_surface_internal_set_user_data(tb->b, TTS_BUFFER_KEY, tb);
	return tb->b;
}

static tdm_test_server_buffer*
get_tts_buffer(tbm_surface_h b)
{
	tdm_test_server_buffer *tb = NULL;
	tbm_surface_internal_get_user_data(b, TTS_BUFFER_KEY, (void **)&tb);
	return tb;
}

static void
exit_test(int sig)
{
	tdm_test_server *data = &tts_data;
	tdm_test_server_output *o = NULL, *oo = NULL;
	tdm_test_server_layer *l = NULL, *ll = NULL;
	tdm_test_server_pp *p = NULL, *pp = NULL;
	tdm_test_server_capture *c = NULL, *cc = NULL;
	tdm_test_server_prop *w = NULL, *ww = NULL;
	int i;

	printf("got signal: %d\n", sig);

	LIST_FOR_EACH_ENTRY_SAFE(o, oo, &data->output_list, link) {
		LIST_DEL(&o->link);

		LIST_FOR_EACH_ENTRY_SAFE(l, ll, &o->layer_list, link) {
			LIST_DEL(&l->link);

			tdm_layer_unset_buffer(l->layer);

			LIST_FOR_EACH_ENTRY_SAFE(w, ww, &l->prop_list, link) {
				LIST_DEL(&w->link);
				free(w);
			}
			for (i = 0; i < TDM_ARRAY_SIZE(l->bufs); i++)
				tbm_surface_destroy(l->bufs[i]);
			free(l);
		}

		tdm_output_commit(o->output, 0, NULL, NULL);

		LIST_FOR_EACH_ENTRY_SAFE(w, ww, &o->prop_list, link) {
			LIST_DEL(&w->link);
			free(w);
		}

		free(o);
	}


	LIST_FOR_EACH_ENTRY_SAFE(p, pp, &data->pp_list, link) {
		LIST_DEL(&p->link);

		tdm_display_lock(data->display);
		tdm_event_loop_source_remove(p->timer_source);
		tdm_display_unlock(data->display);

		tdm_pp_destroy(p->pp);
		for (i = 0; i < TDM_ARRAY_SIZE(p->bufs); i++)
			tbm_surface_destroy(p->bufs[i]);
		free(p);
	}

	LIST_FOR_EACH_ENTRY_SAFE(c, cc, &data->capture_list, link) {
		LIST_DEL(&c->link);
		tdm_capture_destroy(c->capture);
		free(c);
	}

	if (data->display)
		tdm_display_deinit(data->display);

	exit(0);
}

int
main(int argc, char *argv[])
{
	tdm_test_server *data = &tts_data;
	char temp[TDM_SERVER_REPLY_MSG_LEN];
	int len = sizeof temp;
	tdm_error ret;

	signal(SIGINT, exit_test);    /* 2 */
	signal(SIGTERM, exit_test);   /* 15 */

	memset(data, 0, sizeof * data);
	LIST_INITHEAD(&data->output_list);
	LIST_INITHEAD(&data->pp_list);
	LIST_INITHEAD(&data->capture_list);

	/* init value */
	data->bflags = TBM_BO_SCANOUT;
	data->b_fill = PATTERN_SMPTE;

	data->display = tdm_display_init(&ret);
	TDM_EXIT_IF_FAIL(data->display != NULL);

	parse_args(data, argc, argv);
	interpret_args(data);

	if (data->do_query) {
		tdm_helper_get_display_information(data->display, temp, &len);
		printf("%s", temp);
		goto done;
	}

	run_test(data);

done:
	tdm_display_deinit(data->display);

	return 0;
}

static void
output_setup(tdm_test_server_output *o)
{
	tdm_test_server *data = o->data;
	const tdm_output_mode *modes, *found = NULL, *best = NULL, *prefer = NULL;
	tdm_test_server_prop *w = NULL;
	const tdm_prop *props;
	int i, count;
	tdm_error ret;

	o->output = tdm_display_get_output(data->display, o->idx, &ret);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	ret = tdm_output_get_available_modes(o->output, &modes, &count);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	for (i = 0; i < count; i++) {
		if (!strncmp(o->mode, modes[i].name, TDM_NAME_LEN) && o->refresh == modes[i].vrefresh) {
			found = &modes[i];
			printf("found mode: %dx%d %d\n", found->hdisplay, found->vdisplay, found->vrefresh);
			break;
		}
		if (!best)
			best = &modes[i];
		if (modes[i].flags & TDM_OUTPUT_MODE_TYPE_PREFERRED)
			prefer = &modes[i];
	}
	if (!found && prefer) {
		found = prefer;
		printf("found prefer mode: %dx%d %d\n", found->hdisplay, found->vdisplay, found->vrefresh);
	}
	if (!found && best) {
		found = best;
		printf("found best mode: %dx%d %d\n", found->hdisplay, found->vdisplay, found->vrefresh);
	}

	if (!found) {
		printf("couldn't find any mode\n");
		exit(0);
	}

	ret = tdm_output_set_mode(o->output, found);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	printf("output %d: %s %d\n", o->idx, found->name, found->vrefresh);

	ret = tdm_output_get_available_properties(o->output, &props, &count);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	LIST_FOR_EACH_ENTRY(w, &o->prop_list, link) {
		for (i = 0; i < count; i++) {
			if (strncmp(w->name, props[i].name, TDM_NAME_LEN))
				continue;
			ret = tdm_output_set_property(o->output, props[i].id, w->value);
			TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);
			printf("\tprop '%s': %d\n", props[i].name, w->value.u32);
			break;
		}
	}

	/* DPMS on forcely at the first time. */
	ret = tdm_output_set_dpms(o->output, TDM_OUTPUT_DPMS_ON);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);
}

static tbm_surface_h
layer_get_buffer(tdm_test_server_layer *l)
{
	int i, size = TDM_ARRAY_SIZE(l->bufs);
	if (!l->bufs[0]) {
		for (i = 0; i < size; i++) {
			int width = (l->info.src_config.size.h)?:l->info.src_config.pos.w;
			int height = (l->info.src_config.size.v)?:l->info.src_config.pos.h;
			unsigned int format = (l->info.src_config.format)?:DEFAULT_FORMAT;
			int flags = l->o->data->bflags;
			tbm_surface_h b = buffer_allocate(width, height, format, flags);
			TDM_EXIT_IF_FAIL(b != NULL);
			l->bufs[i] = b;
		}
	}
	for (i = 0; i < size; i++) {
		tdm_test_server_buffer *tb = get_tts_buffer(l->bufs[i]);
		TDM_EXIT_IF_FAIL(tb != NULL);
		if (!tb->in_use)
			return l->bufs[i];
	}
	printf("no available layer buffer.\n");
	exit(0);
}

static void
layer_cb_commit(tdm_output *output, unsigned int sequence,
				unsigned int tv_sec, unsigned int tv_usec,
				void *user_data)
{
	tdm_test_server_layer *l = user_data;
	tbm_surface_h b = layer_get_buffer(l);
	TDM_EXIT_IF_FAIL(b != NULL);

	if (!l->is_primary || l->o->fill_primary_layer)
		tdm_test_buffer_fill(b, l->data->b_fill);

	if (!l->is_primary || l->o->fill_primary_layer)
		layer_show_buffer(l, b);
}

static void
layer_cb_buffer_release(tbm_surface_h b, void *user_data)
{
	tdm_test_server_buffer *tb = get_tts_buffer(b);
	tb->in_use = 0;
	tdm_buffer_remove_release_handler(b, layer_cb_buffer_release, NULL);
	if (tb->done)
		tb->done(b, user_data);
}

static void
layer_show_buffer(tdm_test_server_layer *l, tbm_surface_h b)
{
	tdm_test_server *data = l->o->data;
	tdm_test_server_buffer *tb;
	tdm_error ret;

	ret = tdm_layer_set_buffer(l->layer, b);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	tb = get_tts_buffer(b);
	TDM_EXIT_IF_FAIL(tb != NULL);

	tb->in_use = 1;
	tdm_buffer_add_release_handler(b, layer_cb_buffer_release, NULL);

	if (data->do_vblank)
		ret = tdm_output_commit(l->o->output, 0, layer_cb_commit, l);
	else
		ret = tdm_output_commit(l->o->output, 0, NULL, NULL);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	printf("show:\tl(%p) b(%p)\n", l, b);
}

static void
layer_setup(tdm_test_server_layer *l, tbm_surface_h b)
{
	tdm_test_server_prop *w = NULL;
	const tdm_prop *props;
	int i, count;
	tdm_error ret;
	tbm_surface_info_s info;

	l->layer = tdm_output_get_layer(l->o->output, l->idx, &ret);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	/* The size and format information should be same with buffer's */
	tbm_surface_get_info(b, &info);
	if (IS_RGB(info.format)) {
		l->info.src_config.size.h = info.planes[0].stride >> 2;
		l->info.src_config.size.v = info.height;
	} else {
		l->info.src_config.size.h = info.planes[0].stride;
		l->info.src_config.size.v = info.height;
	}
	l->info.src_config.format = info.format;

	ret = tdm_layer_set_info(l->layer, &l->info);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	printf("layer %d: output(%d) ", l->idx, l->o->idx);
	print_config(&l->info.src_config);
	printf(" ! ");
	print_pos(&l->info.dst_pos);
	printf(" transform(%s)\n", tdm_transform_str(l->info.transform));

	ret = tdm_layer_get_available_properties(l->layer, &props, &count);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	LIST_FOR_EACH_ENTRY(w, &l->prop_list, link) {
		for (i = 0; i < count; i++) {
			if (strncmp(w->name, props[i].name, TDM_NAME_LEN))
				continue;
			ret = tdm_layer_set_property(l->layer, props[i].id, w->value);
			TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);
			printf("\tprop '%s': %d\n", props[i].name, w->value.u32);
			break;
		}
	}
}

static tbm_surface_h
pp_get_buffer(tdm_test_server_pp *p)
{
	int i, size = TDM_ARRAY_SIZE(p->bufs);
	if (!p->bufs[0]) {
		for (i = 0; i < size; i++) {
			int width = (p->info.src_config.size.h)?:p->info.src_config.pos.w;
			int height = (p->info.src_config.size.v)?:p->info.src_config.pos.h;
			unsigned int format = (p->info.src_config.format)?:DEFAULT_FORMAT;
			tbm_surface_h b = buffer_allocate(width, height, format, 0);
			TDM_EXIT_IF_FAIL(b != NULL);
			p->bufs[i] = b;
		}
	}
	for (i = 0; i < size; i++) {
		tdm_test_server_buffer *tb = get_tts_buffer(p->bufs[i]);
		if (!tb->in_use) {
			tdm_test_buffer_fill(p->bufs[i], p->data->b_fill);
			return p->bufs[i];
		}
	}
	printf("no available pp buffer.\n");
	exit(0);
}

static void
pp_cb_done(tdm_pp *pp, tbm_surface_h sb, tbm_surface_h db, void *user_data)
{
	tdm_test_server_buffer *stb, *dtb;

	stb = get_tts_buffer(sb);
	TDM_EXIT_IF_FAIL(stb != NULL);

	dtb = get_tts_buffer(db);
	TDM_EXIT_IF_FAIL(dtb != NULL);

	stb->in_use = dtb->in_use = 0;

	layer_show_buffer(dtb->l, db);
}

static void
pp_convert_buffer(tdm_test_server_pp *p, tbm_surface_h sb, tbm_surface_h db)
{
	tdm_test_server_buffer *stb, *dtb;
	tdm_error ret;

	stb = get_tts_buffer(sb);
	TDM_EXIT_IF_FAIL(stb != NULL);

	dtb = get_tts_buffer(db);
	TDM_EXIT_IF_FAIL(dtb != NULL);

	stb->in_use = dtb->in_use = 1;
	dtb->l = p->l;

	ret = tdm_pp_attach(p->pp, sb, db);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	ret = tdm_pp_commit(p->pp);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	printf("convt:\tp(%p) sb(%p) db(%p)\n", p, sb, db);
}

/* tdm_event_loop_xxx() function is not for the display server. It's for TDM
 * backend module. I use them only for simulating the video play. When we call
 * tdm_event_loop_xxx() outside of TDM backend module, we have to lock/unlock
 * the TDM display. And when the callback function of tdm_event_loop_xxx() is
 * called, the display has been already locked. So in this test application,
 * we have to unlock/lock the display for testing.
 */
static tdm_error
pp_cb_timeout(void *user_data)
{
	tdm_test_server_pp *p = user_data;
	tdm_test_server *data = p->l->o->data;
	tbm_surface_h sb, db;
	tdm_error ret;

	tdm_display_unlock(data->display);

	sb = pp_get_buffer(p);
	TDM_EXIT_IF_FAIL(sb != NULL);
	db = layer_get_buffer(p->l);
	TDM_EXIT_IF_FAIL(db != NULL);

	pp_convert_buffer(p, sb, db);

	tdm_display_lock(data->display);
	ret = tdm_event_loop_source_timer_update(p->timer_source, 1000 / p->fps);
	tdm_display_unlock(data->display);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	tdm_display_lock(data->display);

	return TDM_ERROR_NONE;
}

static void
pp_setup(tdm_test_server_pp *p, tbm_surface_h sb, tbm_surface_h db)
{
	tdm_test_server *data = p->l->o->data;
	tbm_surface_info_s info;
	tdm_error ret;

	p->pp = tdm_display_create_pp(data->display, &ret);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	/* The size and format information should be same with buffer's */
	tbm_surface_get_info(sb, &info);
	if (IS_RGB(info.format)) {
		p->info.src_config.size.h = info.planes[0].stride >> 2;
		p->info.src_config.size.v = info.height;
	} else {
		p->info.src_config.size.h = info.planes[0].stride;
		p->info.src_config.size.v = info.height;
	}
	p->info.src_config.format = info.format;

	/* The size and format information should be same with buffer's */
	tbm_surface_get_info(db, &info);
	if (IS_RGB(info.format)) {
		p->info.dst_config.size.h = info.planes[0].stride >> 2;
		p->info.dst_config.size.v = info.height;
	} else {
		p->info.dst_config.size.h = info.planes[0].stride;
		p->info.dst_config.size.v = info.height;
	}
	p->info.dst_config.format = info.format;

	ret = tdm_pp_set_info(p->pp, &p->info);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	ret = tdm_pp_set_done_handler(p->pp, pp_cb_done, NULL);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	printf("pp: ");
	print_config(&p->info.src_config);
	printf(" ! ");
	print_config(&p->info.dst_config);
	printf(" fps(%d) transform(%s)\n", p->fps, tdm_transform_str(p->info.transform));
	if (p->l)
		printf("\toutput_idx(%d) layer_idx(%d)\n", p->l->o->idx, p->l->idx);

	layer_setup(p->l, db);

	/* tdm_event_loop_xxx() function is not for the display server. It's for TDM
	 * backend module. I use them only for simulating the video play. When we call
	 * tdm_event_loop_xxx() outside of TDM backend module, we have to lock/unlock
	 * the TDM display. And when the callback function of tdm_event_loop_xxx() is
	 * called, the display has been already locked. So in this test application,
	 * we have to unlock/lock the display for testing.
	 */
	tdm_display_lock(data->display);
	p->timer_source = tdm_event_loop_add_timer_handler(data->display, pp_cb_timeout, p, &ret);
	tdm_display_unlock(data->display);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	tdm_display_lock(data->display);
	ret = tdm_event_loop_source_timer_update(p->timer_source, 1000 / p->fps);
	tdm_display_unlock(data->display);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);
}

static void
capture_cb_buffer_done(tbm_surface_h b, void *user_data)
{
	tdm_test_server_buffer *tb;

	tb = get_tts_buffer(b);
	TDM_EXIT_IF_FAIL(tb != NULL);

	capture_attach(tb->c, b);
}

static void
capture_cb_done(tdm_capture *capture, tbm_surface_h b, void *user_data)
{
	tdm_test_server_buffer *tb;

	tb = get_tts_buffer(b);
	TDM_EXIT_IF_FAIL(tb != NULL);

	tb->in_use = 0;

	tb->done = capture_cb_buffer_done;
	layer_show_buffer(tb->l, b);
}

static void
capture_attach(tdm_test_server_capture *c, tbm_surface_h b)
{
	tdm_error ret;
	tdm_test_server_buffer *tb;

	tb = get_tts_buffer(b);
	TDM_EXIT_IF_FAIL(tb != NULL);

	tb->in_use = 1;
	tb->l = c->l;
	tb->c = c;

	ret = tdm_capture_attach(c->capture, b);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	printf("capture:\tc(%p) b(%p)\n", c, b);

	ret = tdm_capture_commit(c->capture);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);
}

static void
capture_setup(tdm_test_server_capture *c, tbm_surface_h b)
{
	tdm_test_server *data = c->l->o->data;
	tdm_output *output;
	tdm_layer *layer;
	tbm_surface_info_s info;
	tdm_error ret;

	if (c->output_idx != -1 && c->layer_idx == -1) {
		output = tdm_display_get_output(data->display, c->output_idx, &ret);
		TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);
		c->capture = tdm_output_create_capture(output, &ret);
		TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);
	} else if (c->output_idx != -1 && c->layer_idx != -1) {
		output = tdm_display_get_output(data->display, c->output_idx, &ret);
		TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);
		layer = tdm_output_get_layer(output, c->layer_idx, &ret);
		TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);
		c->capture = tdm_layer_create_capture(layer, &ret);
		TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);
	}

	TDM_EXIT_IF_FAIL(c->capture != NULL);

	/* The size and format information should be same with buffer's */
	tbm_surface_get_info(b, &info);
	if (IS_RGB(info.format)) {
		c->info.dst_config.size.h = info.planes[0].stride >> 2;
		c->info.dst_config.size.v = info.height;
	} else {
		c->info.dst_config.size.h = info.planes[0].stride;
		c->info.dst_config.size.v = info.height;
	}
	c->info.dst_config.format = info.format;

	ret = tdm_capture_set_info(c->capture, &c->info);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	ret = tdm_capture_set_done_handler(c->capture, capture_cb_done, NULL);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	printf("capture: o(%d) l(%d) ", c->output_idx, c->layer_idx);
	print_config(&c->info.dst_config);
	printf(" transform(%s)\n", tdm_transform_str(c->info.transform));
	if (c->l)
		printf("\toutput_idx(%d) layer_idx(%d)\n", c->l->o->idx, c->l->idx);

	layer_setup(c->l, b);
}

static void
run_test(tdm_test_server *data)
{
	tdm_test_server_output *o = NULL;
	tdm_test_server_layer *l = NULL;
	tdm_test_server_pp *p = NULL;
	tdm_test_server_capture *c = NULL;
	tdm_display_capability caps;
	tdm_error ret;

	ret = tdm_display_get_capabilities(data->display, &caps);
	TDM_EXIT_IF_FAIL(ret == TDM_ERROR_NONE);

	LIST_FOR_EACH_ENTRY(o, &data->output_list, link) {
		LIST_FOR_EACH_ENTRY(l, &o->layer_list, link) {
			if (!l->owner_p && !l->owner_c) {
				tbm_surface_h b;
				b = layer_get_buffer(l);
				if (!l->is_primary || l->o->fill_primary_layer)
					tdm_test_buffer_fill(b, data->b_fill);
				layer_setup(l, b);
				layer_show_buffer(l, b);
			}
		}
	}

	LIST_FOR_EACH_ENTRY(p, &data->pp_list, link) {
		tbm_surface_h sb, db;
		TDM_GOTO_IF_FAIL(caps & TDM_DISPLAY_CAPABILITY_PP, no_pp);
		sb = pp_get_buffer(p);
		TDM_EXIT_IF_FAIL(sb != NULL);
		db = layer_get_buffer(p->l);
		TDM_EXIT_IF_FAIL(db != NULL);
		pp_setup(p, sb, db);
		pp_convert_buffer(p, sb, db);
	}

	LIST_FOR_EACH_ENTRY(c, &data->capture_list, link) {
		TDM_GOTO_IF_FAIL(caps & TDM_DISPLAY_CAPABILITY_CAPTURE, no_capture);
		tbm_surface_h b;
		b = layer_get_buffer(c->l);
		capture_setup(c, b);
		capture_attach(c, b);
		b = layer_get_buffer(c->l);
		capture_attach(c, b);
		b = layer_get_buffer(c->l);
		capture_attach(c, b);
	}

	printf("enter test loop\n");

	while (1)
		tdm_display_handle_events(data->display);

	return;
no_pp:
	printf("no PP capability\n");
	exit(0);
no_capture:
	printf("no Capture capability\n");
	exit(0);
}
