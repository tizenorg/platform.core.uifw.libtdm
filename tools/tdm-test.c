/*
Copyright (C) 2015 Samsung Electronics co., Ltd. All Rights Reserved.

Contact:
      Changyeon Lee <cyeon.lee@samsung.com>,
      JunKyeong Kim <jk0430.kim@samsung.com>,
      Boram Park <boram1288.park@samsung.com>,
      SooChan Lim <sc1.lim@samsung.com>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "tdm-test.h"

int tdm_debug = 0;
tdm_test_data g_data;

static output_arg*
find_output(tdm_test_data *data, int output_idx)
{
	output_arg *arg_o;

	LIST_FOR_EACH_ENTRY(arg_o, &data->args.output_list, link) {
		if (arg_o->output_idx == output_idx)
			return arg_o;
	}

	return NULL;
}

static layer_arg*
find_layer(tdm_test_data *data, output_arg *arg_o, int layer_idx)
{
	layer_arg *arg_l;

	LIST_FOR_EACH_ENTRY(arg_l, &arg_o->layer_list, link) {
		if (arg_l->layer_idx == layer_idx)
			return arg_l;
	}

	return NULL;
}

static int
parse_output(tdm_test_data *data, const char *p)
{
	char mode[32];
	output_arg *arg_o = calloc(1, sizeof(output_arg));
	return_val_if_fail(arg_o != NULL, 0);

	if (sscanf(p, "%d:%31s", &arg_o->output_idx, mode) != 2)
		goto fail;

	strncpy(arg_o->mode, mode, TDM_NAME_LEN);
	arg_o->mode[TDM_NAME_LEN - 1] = '\0';

	LIST_ADD(&arg_o->link, &data->args.output_list);

	LIST_INITHEAD(&arg_o->prop_list);
	LIST_INITHEAD(&arg_o->layer_list);

	TDM_DBG("arg_o(%d) mode(%s)", arg_o->output_idx, arg_o->mode);

	return 1;

fail:
	free(arg_o);
	return 0;
}

static int
parse_output_property(tdm_test_data *data, const char *p)
{
	char name[32];
	output_arg *arg_o;
	int output_idx = -1;
	prop_arg *prop = calloc(1, sizeof(prop_arg));
	return_val_if_fail(prop != NULL, 0);

	if (sscanf(p, "%d:%31s:%d", &output_idx, name, &prop->value) != 3)
		goto fail;

	arg_o = find_output(data, output_idx);
	goto_if_fail(arg_o != NULL, fail);

	strncpy(prop->name, name, TDM_NAME_LEN);
	prop->name[TDM_NAME_LEN - 1] = '\0';

	LIST_ADD(&prop->link, &arg_o->prop_list);

	TDM_DBG("arg_o(%d) prop(%s,%d)",
	        arg_o->output_idx, prop->name, prop->value);

	return 1;
fail:
	free(prop);
	return 0;
}

static int
parse_layer(tdm_test_data *data, const char *p)
{
	output_arg *arg_o;
	char *end;
	layer_arg *arg_l = calloc(1, sizeof(layer_arg));
	return_val_if_fail(arg_l != NULL, 0);

	arg_l->output_idx = strtoul(p, &end, 10);
	arg_o = find_output(data, arg_l->output_idx);
	goto_if_fail(arg_o != NULL, fail);

	if (*end == ',') {
		p = end + 1;
		arg_l->layer_idx = strtoul(p, &end, 10);
	} else {
		arg_l->layer_idx = -1;
	}

	goto_if_fail(*end == ':', fail);

	p = end + 1;
	arg_l->w = strtoul(p, &end, 10);
	goto_if_fail(*end == 'x', fail);

	p = end + 1;
	arg_l->h = strtoul(p, &end, 10);

	if (*end == '+' || *end == '-') {
		arg_l->x = strtol(end, &end, 10);
		goto_if_fail(*end == '+' || *end == '-', fail);

		arg_l->y = strtol(end, &end, 10);
	}

	if (*end == '*') {
		p = end + 1;
		arg_l->scale = strtoul(p, &end, 10);
	} else {
		arg_l->scale = 1;
	}

	if (*end == '#') {
		p = end + 1;
		arg_l->transform = strtoul(p, &end, 10);
	}

	if (*end == '@') {
		p = end + 1;
		strncpy(arg_l->format, p, 4);
		arg_l->format[4] = '\0';
	} else {
		strncpy(arg_l->format, "XR24", 4);
		arg_l->format[4] = '\0';
	}

	LIST_ADD(&arg_l->link, &arg_o->layer_list);

	LIST_INITHEAD(&arg_l->prop_list);

	TDM_DBG("arg_o(%d) arg_l(%d) %dx%d+%d+%d*%d#%d@%s",
	        arg_l->output_idx, arg_l->layer_idx, arg_l->w, arg_l->h,
	        arg_l->x, arg_l->y, arg_l->scale, arg_l->transform, arg_l->format);

	return 1;

fail:
	free(arg_l);
	return 0;
}

static int
parse_layer_property(tdm_test_data *data, const char *p)
{
	char *end, *t;
	output_arg *arg_o;
	layer_arg *arg_l;
	int output_idx = -1;
	int layer_idx = -1;
	prop_arg *prop = calloc(1, sizeof(prop_arg));
	return_val_if_fail(prop != NULL, 0);

	output_idx = strtoul(p, &end, 10);
	arg_o = find_output(data, output_idx);
	goto_if_fail(arg_o != NULL, fail);

	if (*end == ',') {
		p = end + 1;
		layer_idx = strtoul(p, &end, 10);
	}

	arg_l = find_layer(data, arg_o, layer_idx);
	goto_if_fail(arg_l != NULL, fail);

	goto_if_fail(*end == ':', fail);

	p = end + 1;

	t = strpbrk(p,":");
	goto_if_fail(t - p < TDM_NAME_LEN, fail);

	snprintf(prop->name, t - p + 1, "%s", p);

	p = t + 1;
	prop->value = strtoul(p, &end, 10);

	LIST_ADD(&prop->link, &arg_l->prop_list);

	TDM_DBG("output(%d) layer(%d) prop(%s,%d)",
	        arg_l->output_idx, arg_l->layer_idx, prop->name, prop->value);

	return 1;
fail:
	free(prop);
	return 0;
}

static int
parse_pp(tdm_test_data *data, const char *p)
{
	char *end;
	pp_arg *arg_p;

	return_val_if_fail(data->args.pp == NULL, 0);

	arg_p = calloc(1, sizeof(pp_arg));
	return_val_if_fail(arg_p != NULL, 0);

	arg_p->src_size_h = strtoul(p, &end, 10);
	goto_if_fail(*end == 'x', fail);

	p = end + 1;
	arg_p->src_size_v = strtoul(p, &end, 10);

	if (*end == ',') {
		p = end + 1;
		arg_p->src_pos_w = strtoul(p, &end, 10);
		goto_if_fail(*end == 'x', fail);

		p = end + 1;
		arg_p->src_pos_h = strtoul(p, &end, 10);

		if (*end == '+') {
			arg_p->src_pos_x = strtol(end, &end, 10);
			goto_if_fail(*end == '+', fail);

			arg_p->src_pos_y = strtol(end, &end, 10);
		}
	} else {
		arg_p->src_pos_x = 0;
		arg_p->src_pos_y = 0;
		arg_p->src_pos_w = arg_p->src_size_h;
		arg_p->src_pos_h = arg_p->src_size_v;
	}

	if (*end == '@') {
		p = end + 1;
		strncpy(arg_p->src_format, p, 4);
		arg_p->src_format[4] = '\0';
	} else {
		strncpy(arg_p->src_format, "XR24", 4);
		arg_p->src_format[4] = '\0';
	}

	goto_if_fail(*end == ':', fail);

	p = end + 1;
	arg_p->dst_size_h = strtoul(p, &end, 10);
	goto_if_fail(*end == 'x', fail);

	p = end + 1;
	arg_p->dst_size_v = strtoul(p, &end, 10);

	if (*end == ',') {
		p = end + 1;
		arg_p->dst_pos_w = strtoul(p, &end, 10);
		goto_if_fail(*end == 'x', fail);

		p = end + 1;
		arg_p->dst_pos_h = strtoul(p, &end, 10);

		if (*end == '+') {
			arg_p->dst_pos_x = strtol(end, &end, 10);
			goto_if_fail(*end == '+', fail);

			arg_p->dst_pos_y = strtol(end, &end, 10);
		}
	} else {
		arg_p->dst_pos_x = 0;
		arg_p->dst_pos_y = 0;
		arg_p->dst_pos_w = arg_p->dst_size_h;
		arg_p->dst_pos_h = arg_p->dst_size_v;
	}

	if (*end == '@') {
		p = end + 1;
		strncpy(arg_p->dst_format, p, 4);
		arg_p->dst_format[4] = '\0';
	} else {
		strncpy(arg_p->dst_format, "XR24", 4);
		arg_p->dst_format[4] = '\0';
	}

	if (*end == '#') {
		p = end + 1;
		arg_p->transform = strtoul(p, &end, 10);
	}

	if (*end == '-') {
		p = end + 1;
		strncpy(arg_p->filename, p, TDM_NAME_LEN);
		arg_p->filename[TDM_NAME_LEN] = '\0';
		arg_p->has_filename = 1;
	}

	LIST_INITHEAD(&arg_p->prop_list);

	data->args.pp = arg_p;

	if (arg_p->has_filename) {
		TDM_DBG("pp %dx%d(%dx%d+%d+%d)@%s => %dx%d(%dx%d+%d+%d)@%s #%d-%s",
		        arg_p->src_size_h, arg_p->src_size_v, arg_p->src_pos_x, arg_p->src_pos_y,
		        arg_p->src_pos_w, arg_p->src_pos_h, arg_p->src_format,
		        arg_p->dst_size_h, arg_p->dst_size_v, arg_p->dst_pos_x, arg_p->dst_pos_y,
		        arg_p->dst_pos_w, arg_p->dst_pos_h, arg_p->dst_format,
		        arg_p->transform, arg_p->filename);
	} else {
		TDM_DBG("pp %dx%d(%dx%d+%d+%d)@%s => %dx%d(%dx%d+%d+%d)@%s #%d",
		        arg_p->src_size_h, arg_p->src_size_v, arg_p->src_pos_x, arg_p->src_pos_y,
		        arg_p->src_pos_w, arg_p->src_pos_h, arg_p->src_format,
		        arg_p->dst_size_h, arg_p->dst_size_v, arg_p->dst_pos_x, arg_p->dst_pos_y,
		        arg_p->dst_pos_w, arg_p->dst_pos_h, arg_p->dst_format,
		        arg_p->transform);
	}

	return 1;

fail:
	free(arg_p);
	return 0;
}

static int
parse_pp_property(tdm_test_data *data, const char *p)
{
	char *end, *t;
	prop_arg *prop;

	return_val_if_fail(data->args.pp != NULL, 0);

	prop = calloc(1, sizeof(prop_arg));
	return_val_if_fail(prop != NULL, 0);

	t = strpbrk(p,":");
	goto_if_fail(t - p < TDM_NAME_LEN, fail);

	snprintf(prop->name, t - p + 1, "%s", p);

	p = t + 1;
	prop->value = strtoul(p, &end, 10);

	LIST_ADD(&prop->link, &data->args.pp->prop_list);

	TDM_DBG("pp prop(%s,%d)", prop->name, prop->value);

	return 1;
fail:
	free(prop);
	return 0;
}

static int
parse_capture(tdm_test_data *data, const char *p)
{
	char *end;
	capture_arg *arg_c;

	return_val_if_fail(data->args.capture == NULL, 0);

	arg_c = calloc(1, sizeof(capture_arg));
	return_val_if_fail(arg_c != NULL, 0);

	arg_c->output_idx = strtoul(p, &end, 10);

	if (*end == ',') {
		p = end + 1;
		arg_c->layer_idx = strtoul(p, &end, 10);
	} else {
		arg_c->layer_idx = -1;
	}

	goto_if_fail(*end == ':', fail);

	p = end + 1;
	arg_c->size_h = strtoul(p, &end, 10);
	goto_if_fail(*end == 'x', fail);

	p = end + 1;
	arg_c->size_v = strtoul(p, &end, 10);

	if (*end == ',') {
		p = end + 1;
		arg_c->pos_w = strtoul(p, &end, 10);
		goto_if_fail(*end == 'x', fail);

		p = end + 1;
		arg_c->pos_h = strtoul(p, &end, 10);

		if (*end == '+') {
			arg_c->pos_x = strtol(end, &end, 10);
			goto_if_fail(*end == '+', fail);

			arg_c->pos_y = strtol(end, &end, 10);
		}
	} else {
		arg_c->pos_x = 0;
		arg_c->pos_y = 0;
		arg_c->pos_w = arg_c->size_h;
		arg_c->pos_h = arg_c->size_v;
	}

	if (*end == '@') {
		p = end + 1;
		strncpy(arg_c->format, p, 4);
		arg_c->format[4] = '\0';
	} else {
		strncpy(arg_c->format, "XR24", 4);
		arg_c->format[4] = '\0';
	}

	if (*end == '#') {
		p = end + 1;
		arg_c->transform = strtoul(p, &end, 10);
	}

	if (*end == '-') {
		p = end + 1;
		strncpy(arg_c->filename, p, TDM_NAME_LEN);
		arg_c->filename[TDM_NAME_LEN] = '\0';
		arg_c->has_filename = 1;
	}

	LIST_INITHEAD(&arg_c->prop_list);

	data->args.capture = arg_c;

	if (arg_c->has_filename) {
		TDM_DBG("capture output(%d) layer(%d) %dx%d(%dx%d+%d+%d)@%s #%d-%s",
		        arg_c->output_idx, arg_c->layer_idx,
		        arg_c->size_h, arg_c->size_v, arg_c->pos_x, arg_c->pos_y,
		        arg_c->pos_w, arg_c->pos_h, arg_c->format,
		        arg_c->transform, arg_c->filename);
	} else {
		TDM_DBG("capture output(%d) layer(%d) %dx%d(%dx%d+%d+%d)@%s #%d",
		        arg_c->output_idx, arg_c->layer_idx,
		        arg_c->size_h, arg_c->size_v, arg_c->pos_x, arg_c->pos_y,
		        arg_c->pos_w, arg_c->pos_h, arg_c->format,
		        arg_c->transform);
	}

	return 1;

fail:
	free(arg_c);
	return 0;
}

static int
parse_capture_property(tdm_test_data *data, const char *p)
{
	char *end, *t;
	prop_arg *prop;

	return_val_if_fail(data->args.capture != NULL, 0);

	prop = calloc(1, sizeof(prop_arg));
	return_val_if_fail(prop != NULL, 0);

	t = strpbrk(p,":");
	goto_if_fail(t - p < TDM_NAME_LEN, fail);

	snprintf(prop->name, t - p + 1, "%s", p);

	p = t + 1;
	prop->value = strtoul(p, &end, 10);

	LIST_ADD(&prop->link, &data->args.capture->prop_list);

	TDM_DBG("capture prop(%s,%d)", prop->name, prop->value);

	return 1;
fail:
	free(prop);
	return 0;
}

static void
signal_handler(int signal)
{
	TDM_DBG("\t++ killed by signal(%d).", signal);
	exit(1);
}

static void
usage(tdm_test_data *data)
{
	printf("usage: %s\n", data->cmd_name);
	printf("\n");
	printf(" Test options:\n");
	printf("\n");
	printf("   -a\tset all layer objects for all connected outputs\n");
	printf("   -o\tset a mode for a output object\n");
	printf("     \t  <output_idx>:<mode>\n");
	printf("   -O\tset property for a output object\n");
	printf("     \t  <output_idx>:<prop_name>:<value>\n");
	printf("   -l\tset a layer object (use primary layer if layer_idx skipped)\n");
	printf("     \t  <output_idx>[,<layer_idx>]:<w>x<h>[+<x>+<y>][*<scale>][#<transform>][@<format>]\n");
	printf("   -L\tset property for a layer object (Use primary layer if layer_idx skipped)\n");
	printf("     \t  <output_idx>[,<layer_idx>]:<prop_name>:<value>\n");
	printf("\n");
	printf(" PP options: can be used with -l to display the result\n");
	printf("\n");
	printf("   -p\tset a PP object\n");
	printf("     \t  <w>x<h>[,<w>x<h>[+<x>+<y>]][@<format>]:<w>x<h>[,<w>x<h>[+<x>+<y>]][@<format>][#<transform>][-<filename.png>]\n");
	printf("   -P\tset property for a PP object\n");
	printf("     \t  <prop_name>:<value>\n");
	printf("\n");
	printf(" Capture options: can be used with -l to display the result\n");
	printf("\n");
	printf("   -c\tcatpure a output object or a layer object\n");
	printf("     \t  <output_idx>[,<layer_idx>]:<w>x<h>[,<w>x<h>[+<x>+<y>]][@<format>][#<transform>][-<filename.png>]\n");
	printf("   -C\tset property for a capture object\n");
	printf("     \t  <prop_name>:<value>\n");
	printf("\n");
//	printf(" General option:\n");
//	printf("\n");
//	printf("   -G\tdrawing with gl\n");
//	printf("   -V\ttest with vsync\n");
//	printf("   -Q\tusing buffer queue with -V\n");
//	printf("\n");
	printf(" Default is to query all information.\n\n");

	tdm_test_display_deinit(data);

	exit(0);
}

static char optstr[] = "ao:O:l:L:p:P:c:C:GVQ";

int
main(int argc, char *argv[])
{
	tdm_test_data *data = &g_data;
	const char *debug;
	int c;

	debug = getenv("TDM_DEBUG");
	if (debug && (strstr(debug, "1")))
		tdm_debug = 1;

	memset(data, 0, sizeof(tdm_test_data));
	LIST_INITHEAD(&data->args.output_list);

	strncpy(data->cmd_name, argv[0], TDM_NAME_LEN);
	data->cmd_name[TDM_NAME_LEN - 1] = '\0';

	signal(SIGSEGV, signal_handler);

	if (argc == 1) {
		tdm_test_display_init(data);
		tdm_test_display_print_infomation(data);
		tdm_test_display_deinit(data);
		exit(0);
	}

	opterr = 0;
	while ((c = getopt(argc, argv, optstr)) != -1) {
		switch (c) {
		case 'a':
			data->args.all_enable = 1;
			break;
		case 'o':
			if (!parse_output(data, optarg))
				usage(data);
			break;
		case 'O':
			if (!parse_output_property(data, optarg))
				usage(data);
			break;
		case 'l':
			if (!parse_layer(data, optarg))
				usage(data);
			break;
		case 'L':
			if (!parse_layer_property(data, optarg))
				usage(data);
			break;
		case 'p':
			if (!parse_pp(data, optarg))
				usage(data);
			break;
		case 'P':
			if (!parse_pp_property(data, optarg))
				usage(data);
			break;
		case 'c':
			if (!parse_capture(data, optarg))
				usage(data);
			break;
		case 'C':
			if (!parse_capture_property(data, optarg))
				usage(data);
			break;
		case 'G':
			data->args.gl_enable = 1;
			break;
		case 'V':
			data->args.vsync_enable = 1;
			break;
		case 'Q':
			data->args.queue_enable = 1;
			break;
		default:
			usage(data);
			break;
		}
	}

	if (!tdm_test_display_init(data))
		return 0;

	tdm_test_display_deinit(data);

	return 0;
}
