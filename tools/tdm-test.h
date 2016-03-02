#ifndef __TDM_TEST_H__
#define __TDM_TEST_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#include <tdm.h>
#include <tdm_list.h>
#include <tdm_log.h>
#include <tbm_bufmgr.h>

#define C(b,m)              (((b) >> (m)) & 0xFF)
#define B(c,s)              (((unsigned int)(c)) & 0xff) << (s))
#define FOURCC(a,b,c,d)     (B(d,24) | B(c,16) | B(b,8) | B(a,0))
#define FOURCC_STR(id)      C(id,0), C(id,8), C(id,16), C(id,24)
#define IS_RGB(f)           ((f) == TBM_FORMAT_XRGB8888 || (f) == TBM_FORMAT_ARGB8888)

#define return_if_fail(cond) {\
    if (!(cond)) {\
        printf("%s(%d): '%s' failed\n", __FUNCTION__, __LINE__, #cond);\
        return;\
    }\
}
#define return_val_if_fail(cond, val) {\
    if (!(cond)) {\
        printf("%s(%d): '%s' failed\n", __FUNCTION__, __LINE__, #cond);\
        return val;\
    }\
}
#define warning_if_fail(cond)  {\
    if (!(cond))\
        printf("%s(%d): '%s' failed\n", __FUNCTION__, __LINE__, #cond);\
}
#define goto_if_fail(cond, dst) {\
    if (!(cond)) {\
        printf("%s(%d): '%s' failed\n", __FUNCTION__, __LINE__, #cond);\
        goto dst;\
    }\
}

typedef struct _prop_arg
{
	char name[TDM_NAME_LEN];
	int value;

	struct list_head link;
} prop_arg;

typedef struct _capture_arg
{
	int output_idx;
	int layer_idx;

	int size_h, size_v;
	int pos_x, pos_y, pos_w, pos_h;
	char format[TDM_NAME_LEN];

	int transform;

	char filename[TDM_NAME_LEN];
	int has_filename;

	tdm_capture *capture;

	struct list_head prop_list;
} capture_arg;

typedef struct _pp_arg
{
	int src_size_h, src_size_v;
	int src_pos_x, src_pos_y, src_pos_w, src_pos_h;
	char src_format[TDM_NAME_LEN];

	int dst_size_h, dst_size_v;
	int dst_pos_x, dst_pos_y, dst_pos_w, dst_pos_h;
	char dst_format[TDM_NAME_LEN];

	int transform;

	char filename[TDM_NAME_LEN];
	int has_filename;

	tdm_pp *pp;

	struct list_head prop_list;
} pp_arg;

typedef struct _layer_arg
{
	int layer_idx;
	int output_idx;
	int x, y, w, h;
	int scale;
	int transform;
	char format[TDM_NAME_LEN];
	struct list_head prop_list;

	tdm_layer *layer;

	struct list_head link;
} layer_arg;

typedef struct _output_arg
{
	int output_idx;
	char mode[TDM_NAME_LEN];
	struct list_head prop_list;
	struct list_head layer_list;

	tdm_output *output;

	struct list_head link;
} output_arg;

typedef struct _tdm_test_data
{
	char cmd_name[TDM_NAME_LEN];

	struct {
		int all_enable;

		struct list_head output_list;
		pp_arg *pp;
		capture_arg *capture;
		int gl_enable;
		int vsync_enable;
		int queue_enable;
	} args;

	tdm_display *display;
	int loop_running;
} tdm_test_data;

int
tdm_test_init(tdm_test_data *data);
void
tdm_test_deinit(tdm_test_data *data);
void
tdm_test_run(tdm_test_data *data);
void
tdm_test_print_infomation(tdm_test_data *data);

int
tdm_test_output_set(tdm_test_data *data, output_arg *arg_o);
int
tdm_test_layer_set(tdm_test_data *data, layer_arg *arg_l);
int
tdm_test_layer_show_buffer(tdm_test_data *data, output_arg *arg_o,
                           layer_arg *arg_l, tbm_surface_h buffer,
                           tdm_output_commit_handler func, void *user_data);

void
tdm_test_buffer_fill(tbm_surface_h buffer);

#endif
