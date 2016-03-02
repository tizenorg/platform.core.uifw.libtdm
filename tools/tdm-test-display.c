#include "tdm-test.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct type_name {
	int type;
	const char *name;
};

#define type_name_fn(res) \
const char * res##_str(int type) {			\
	unsigned int i;					\
	for (i = 0; i < ARRAY_SIZE(res##_names); i++) { \
		if (res##_names[i].type == type)	\
			return res##_names[i].name;	\
	}						\
	return "(invalid)";				\
}

#define bit_name_fn(res)					\
const char * res##_str(int type) {				\
	unsigned int i;						\
	const char *sep = "";					\
	for (i = 0; i < ARRAY_SIZE(res##_names); i++) {		\
		if (type & (1 << i)) {				\
			printf("%s%s", sep, res##_names[i]);	\
			sep = ", ";				\
		}						\
	}							\
	return NULL;						\
}

struct type_name output_status_names[] = {
	{ TDM_OUTPUT_CONN_STATUS_CONNECTED, "connected" },
	{ TDM_OUTPUT_CONN_STATUS_DISCONNECTED, "disconnected" },
	{ TDM_OUTPUT_CONN_STATUS_MODE_SETTED, "unknown" },
};

struct type_name output_type_names[] = {
	{ TDM_OUTPUT_TYPE_Unknown, "unknown" },
	{ TDM_OUTPUT_TYPE_VGA, "VGA" },
	{ TDM_OUTPUT_TYPE_DVII, "DVI-I" },
	{ TDM_OUTPUT_TYPE_DVID, "DVI-D" },
	{ TDM_OUTPUT_TYPE_DVIA, "DVI-A" },
	{ TDM_OUTPUT_TYPE_Composite, "composite" },
	{ TDM_OUTPUT_TYPE_SVIDEO, "s-video" },
	{ TDM_OUTPUT_TYPE_LVDS, "LVDS" },
	{ TDM_OUTPUT_TYPE_Component, "component" },
	{ TDM_OUTPUT_TYPE_9PinDIN, "9-pin DIN" },
	{ TDM_OUTPUT_TYPE_DisplayPort, "DP" },
	{ TDM_OUTPUT_TYPE_HDMIA, "HDMI-A" },
	{ TDM_OUTPUT_TYPE_HDMIB, "HDMI-B" },
	{ TDM_OUTPUT_TYPE_TV, "TV" },
	{ TDM_OUTPUT_TYPE_eDP, "eDP" },
	{ TDM_OUTPUT_TYPE_VIRTUAL, "Virtual" },
	{ TDM_OUTPUT_TYPE_DSI, "DSI" },
};

static type_name_fn(output_status)
static type_name_fn(output_type)

static const char *layer_capa_names[] = {
	"cursor",
	"primary",
	"overlay",
	"unknown",
	"graphic",
	"video",
	"unknown",
	"unknown",
	"scale",
	"transform",
	"scanout",
};

static const char *mode_type_names[] = {
	"builtin",
	"clock_c",
	"crtc_c",
	"preferred",
	"default",
	"userdef",
	"driver",
};

static const char *mode_flag_names[] = {
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

static const char *pp_capa_names[] = {
	"sync",
	"async",
	"unknown",
	"unknown",
	"scale",
	"transform",
};

static const char *capture_capa_names[] = {
	"output",
	"layer",
	"unknown",
	"unknown",
	"scale",
	"transform",
};

static bit_name_fn(mode_type)
static bit_name_fn(mode_flag)
static bit_name_fn(layer_capa)
static bit_name_fn(pp_capa)
static bit_name_fn(capture_capa)

static void
_print_layer_info(tdm_output *output)
{
	tdm_layer *layer;
	tdm_layer_capability capa;
	const tbm_format *formats;
	const tdm_prop *props;
	tdm_error ret = TDM_ERROR_NONE;
	int count, count_format, count_prop;
	int i, j;
	unsigned int zpos;

	count = 0;
	ret = tdm_output_get_layer_count(output, &count);
	return_if_fail(ret == TDM_ERROR_NONE);

	for (i = 0; i < count; i++) {
		layer = (tdm_layer *)tdm_output_get_layer(output, i, &ret);
		return_if_fail(ret == TDM_ERROR_NONE);

		printf("\tlayer index %d:\n", i);

		/* zpos */
		ret = tdm_layer_get_zpos(layer, &zpos);
		return_if_fail(ret == TDM_ERROR_NONE);
		printf("\t\tzpos: %d\n", zpos);

		/* capability */
		ret = tdm_layer_get_capabilities(layer, &capa);
		return_if_fail(ret == TDM_ERROR_NONE);
		printf("\t\tcapability: ");
		layer_capa_str(capa);
		printf("\n");

		/* format */
		count_format = 0;
		ret = tdm_layer_get_available_formats(layer, &formats, &count_format);
		return_if_fail(ret == TDM_ERROR_NONE);
		printf("\t\tformats:");
		for (j = 0; j < count_format; j++)
			printf(" %c%c%c%c", FOURCC_STR(formats[j]));
		printf("\n");

		/* property */
		count_prop = 0;
		ret = tdm_layer_get_available_properties(layer, &props, &count_prop);
		return_if_fail(ret == TDM_ERROR_NONE);
		printf("\t\tprops:");
		for (j = 0; j < count_prop; j++)
			printf (" %s(%d)", props[j].name, props[j].id);
		printf("\n");
	}
}

static void
_print_info(tdm_display *display)
{
	tdm_output *output = NULL;
	tdm_output_type type;
	const tdm_output_mode *modes;
	const tdm_prop *props;
	tdm_output_conn_status status;;
	tdm_error ret = TDM_ERROR_NONE;
	int i, j, count_output, count_mode, count_prop;
	int min_w, min_h, max_w, max_h, preferred_align;
	unsigned int phy_w, phy_h;
	unsigned int subpixel, pipe;

	count_output = 0;
	type = TDM_OUTPUT_TYPE_Unknown;

	ret = tdm_display_get_output_count(display, &count_output);
	return_if_fail(ret == TDM_ERROR_NONE);

	for (i = 0; i < count_output; i++) {
		output = (tdm_output *)tdm_display_get_output(display, i, &ret);
		return_if_fail(ret == TDM_ERROR_NONE);
		printf("output index %d:\n", i);

		/* type */
		ret = tdm_output_get_output_type(output, &type);
		return_if_fail(ret == TDM_ERROR_NONE);
		printf("\ttype: %s\n", output_type_str(type));

		/* status */
		ret = tdm_output_get_conn_status(output, &status);
		return_if_fail(ret == TDM_ERROR_NONE);
		printf("\tstatus: %s\n", output_status_str(status));

		/* mode */
		count_mode = 0;
		ret = tdm_output_get_available_modes(output, &modes, &count_mode);
		return_if_fail(ret == TDM_ERROR_NONE);
		printf ("\tmodes:\n");
		printf("\t\tname refresh (Hz) hdisp hss hse htot vdisp "
		       "vss vse vtot\n");
		for (j = 0; j < count_mode; j++) {
			printf("\t\t%s %d %d %d %d %d %d %d %d %d",
			       modes[j].name, modes[j].vrefresh, modes[j].hdisplay,
			       modes[j].hsync_start, modes[j].hsync_end, modes[j].htotal,
			       modes[j].vdisplay, modes[j].vsync_start, modes[j].vsync_end,
			       modes[j].vtotal);
			printf(" flags: ");
			mode_flag_str(modes[j].flags);
			printf("; type: ");
			mode_type_str(modes[j].type);
			printf("\n");
		}

		/*property */
		count_prop = 0;
		ret = tdm_output_get_available_properties(output, &props, &count_prop);
		printf ("\tprops:");
		for (j = 0; j < count_prop; j++)
			printf (" %s(%d)", props[j].name, props[j].id);
		printf("\n");

		/* size */
		min_w = min_h = max_w = max_h = preferred_align = 0;
		ret = tdm_output_get_available_size(output, &min_w, &min_h, &max_w, &max_h,
		                                    &preferred_align);
		return_if_fail(ret == TDM_ERROR_NONE);
		printf("\tsize: min(%dx%d), max(%d,%d), align(%d)\n",
		       min_w, min_h, max_w, max_h, preferred_align);

		ret = tdm_output_get_physical_size(output, &phy_w, &phy_h);
		return_if_fail(ret == TDM_ERROR_NONE);
		printf("\tphysical: %u mm x %u mm\n", phy_w, phy_h);

		/* subpixel */
		ret = tdm_output_get_subpixel(output, &subpixel);
		return_if_fail(ret == TDM_ERROR_NONE);
		printf("\tsubpixel: %u\n", subpixel);

		/* tdm_output_get_pipe */
		ret = tdm_output_get_pipe(output, &pipe);
		return_if_fail(ret == TDM_ERROR_NONE);
		printf("\tpipe: %u\n", pipe);

		_print_layer_info(output);

		printf("\n");
	}
}

static void
_printf_pp_info(tdm_display *display)
{
	const tbm_format *formats;
	tdm_pp_capability capa = 0;
	tdm_error ret = TDM_ERROR_NONE;
	int format_cnt = 0;
	int min_w, min_h, max_w, max_h, preferred_align;
	int i;

	ret = tdm_display_get_pp_available_formats(display, &formats, &format_cnt);
	return_if_fail(ret == TDM_ERROR_NONE);

	min_w = min_h = max_w = max_h = preferred_align = 0;
	ret = tdm_display_get_pp_available_size(display, &min_w, &min_h, &max_w,
	                                        &max_h, &preferred_align);
	return_if_fail(ret == TDM_ERROR_NONE);

	printf("\tpp:\n");
	printf("\t\tformats: ");
	for (i = 0; i < format_cnt; i++)
		printf(" %c%c%c%c", FOURCC_STR(formats[i]));
	printf("\n");

	printf("\t\tsize: min(%dx%d) max(%dx%d) preferred_align(%d)\n",
	       min_w, min_h, max_w, max_h, preferred_align);

	ret = tdm_display_get_pp_capabilities(display, &capa);
	return_if_fail(ret == TDM_ERROR_NONE);

	printf("\t\tcapability: ");
	pp_capa_str(capa);
	printf("\n");
}

static void
_print_capture_info(tdm_display *display)
{
	const tbm_format *formats;
	tdm_capture_capability capa = 0;
	tdm_error ret = TDM_ERROR_NONE;
	int format_cnt = 0;
	int i;

	ret = tdm_display_get_catpure_available_formats(display, &formats,
	                &format_cnt);
	return_if_fail(ret == TDM_ERROR_NONE);

	printf("\tcapture:\n");
	printf("\t\tformats: ");
	for (i = 0; i < format_cnt; i++)
		printf(" %c%c%c%c", FOURCC_STR(formats[i]));
	printf("\n");

	ret = tdm_display_get_capture_capabilities(display, &capa);
	return_if_fail(ret == TDM_ERROR_NONE);

	printf("\t\tcapability: ");
	capture_capa_str(capa);
	printf("\n");
}

static void
_print_capability_info(tdm_display *display)
{
	tdm_error ret = TDM_ERROR_NONE;
	tdm_display_capability capa = 0;

	ret = tdm_display_get_capabilities(display, &capa);
	return_if_fail(ret == TDM_ERROR_NONE);

	printf("display capability:\n");
	if (capa & TDM_DISPLAY_CAPABILITY_PP)
		_printf_pp_info(display);

	if (capa & TDM_DISPLAY_CAPABILITY_CAPTURE) 
		_print_capture_info(display); 
}

int
tdm_test_display_init(tdm_test_data *data)
{
	tdm_error ret = TDM_ERROR_NONE;

	data->display = tdm_display_init(&ret);
	return_val_if_fail(ret == TDM_ERROR_NONE, 0);

	return 1;
}

void
tdm_test_display_deinit(tdm_test_data *data)
{
	if (data->display) {
		tdm_display_deinit(data->display);
		data->display = NULL;
	}
}

void
tdm_test_display_print_infomation(tdm_test_data *data)
{
	_print_capability_info(data->display);
	_print_info(data->display);
}
