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

#include "tdm.h"
#include "tdm_private.h"
#include "tdm_list.h"

/* CAUTION:
 * - tdm vblank doesn't care about thread things.
 * - DO NOT use the TDM internal functions here.
 *     However, the internal function which does lock/unlock the mutex of
 *     private_display in itself can be called.
 * - DO NOT use the tdm_private_display structure here.
 */

/* TDM vblank
 * - aligned by HW vblank
 * - use a tdm_event_loop_source object only.
 */

#define VER(fmt,arg...)   TDM_ERR("[%p] "fmt, private_vblank, ##arg)
#define VWR(fmt,arg...)   TDM_WRN("[%p] "fmt, private_vblank, ##arg)
#define VIN(fmt,arg...)   TDM_INFO("[%p] "fmt, private_vblank, ##arg)
#define VDB(fmt,arg...)   TDM_DBG("[%p] "fmt, private_vblank, ##arg)

typedef struct _tdm_vblank_wait_info tdm_vblank_wait_info;

typedef struct _tdm_private_vblank {
	struct list_head link;

	tdm_display *dpy;
	tdm_output *output;
	tdm_output_dpms dpms;
	unsigned int vrefresh;

	unsigned int check_HW_or_SW;
	unsigned int fps;
	int offset;
	unsigned int enable_fake;

	double vblank_gap;
	unsigned int last_seq;
	unsigned int last_tv_sec;
	unsigned int last_tv_usec;

	/* for HW */
	double HW_vblank_gap;
	unsigned int HW_enable;
	unsigned int HW_quotient;
	struct list_head HW_wait_list;

	/* for SW */
	tdm_event_loop_source *SW_timer;
	struct list_head SW_pending_wait_list;
	struct list_head SW_wait_list;
#if 0
	tdm_vblank_wait_info *SW_align_wait;
	double SW_align_offset;
	unsigned int SW_align_sec;
	unsigned int SW_align_usec;
#endif
} tdm_private_vblank;

struct _tdm_vblank_wait_info {
	struct list_head link;
	struct list_head valid_link;

	unsigned int stamp;

	unsigned int req_sec;
	unsigned int req_usec;
	unsigned int interval;

	tdm_vblank_handler func;
	void *user_data;
	tdm_private_vblank *private_vblank;

	/* target_sec can be 0 when last_tv_sec is 0 because we can't calculate
	 * target_sec without last_tv_sec. So we have to call tdm_output_wait_vblank
	 * to fill last_tv_sec at the first time.
	 */
	unsigned int target_sec;
	unsigned int target_usec;
	unsigned int target_seq;
	int target_hw_interval;
};

static struct list_head vblank_list;
static struct list_head valid_wait_list;
static unsigned int vblank_list_inited;
static unsigned int stamp = 0;

static tdm_error _tdm_vblank_wait_SW(tdm_vblank_wait_info *wait_info);
#if 0
static void _tdm_vblank_sw_timer_align(tdm_private_vblank *private_vblank);
#endif

#if 0
static void
_print_list(struct list_head *list)
{
	tdm_vblank_wait_info *w = NULL;

	LIST_FOR_EACH_ENTRY(w, list, link) {
		printf(" %d", w->interval);
	}
	printf("\n");
}
#endif

static inline unsigned int
_tdm_vblank_check_valid(tdm_vblank_wait_info *wait_info)
{
	tdm_vblank_wait_info *w = NULL;

	if (!wait_info)
		return 0;

	LIST_FOR_EACH_ENTRY(w, &valid_wait_list, valid_link) {
		if (w->stamp == wait_info->stamp)
			return 1;
	}

	return 0;
}

static inline unsigned int
_tdm_vblank_find_wait(tdm_vblank_wait_info *wait_info, struct list_head *list)
{
	tdm_vblank_wait_info *w = NULL;

	if (!wait_info)
		return 0;

	LIST_FOR_EACH_ENTRY(w, list, link) {
		if (w->stamp == wait_info->stamp)
			return 1;
	}

	return 0;
}

static inline void
_tdm_vblank_insert_wait(tdm_vblank_wait_info *wait_info, struct list_head *list)
{
	tdm_vblank_wait_info *w = NULL;

	LIST_FOR_EACH_ENTRY_REV(w, list, link) {
		/* If last_tv_sec == 0, we can't calculate target_sec. */
		if (wait_info->target_sec == 0) {
			if (w->interval > wait_info->interval)
				continue;
			list = &w->link;
			break;
		} else {
			if (w->target_sec > wait_info->target_sec)
				continue;
			if (w->target_usec > wait_info->target_usec)
				continue;
			list = &w->link;
		}
		break;
	}

	LIST_ADDTAIL(&wait_info->link, list->next);
}

static void
_tdm_vblank_change_to_SW(tdm_private_vblank *private_vblank)
{
	tdm_vblank_wait_info *w = NULL, *ww = NULL;

	VIN("Change to SW");

	LIST_FOR_EACH_ENTRY_SAFE(w, ww, &private_vblank->HW_wait_list, link) {
		LIST_DEL(&w->link);
		_tdm_vblank_wait_SW(w);
	}
}

static void
_tdm_vblank_free_HW_wait(tdm_private_vblank *private_vblank, tdm_error error, unsigned int call_cb)
{
	tdm_vblank_wait_info *w = NULL, *ww = NULL;

	LIST_FOR_EACH_ENTRY_SAFE(w, ww, &private_vblank->HW_wait_list, link) {
		LIST_DEL(&w->link);
		LIST_DEL(&w->valid_link);

		if (call_cb && w->func)
			w->func(private_vblank, error, 0, 0, 0, w->user_data);

		free(w);
	}
}

static void
_tdm_vblank_cb_output_change(tdm_output *output, tdm_output_change_type type,
							 tdm_value value, void *user_data)
{
	tdm_private_vblank *private_vblank = user_data;

	TDM_RETURN_IF_FAIL(private_vblank != NULL);

	switch (type) {
	case TDM_OUTPUT_CHANGE_DPMS:
		if (private_vblank->dpms == value.u32)
			break;
		VIN("dpms %s", tdm_dpms_str(value.u32));
		private_vblank->dpms = value.u32;
		private_vblank->check_HW_or_SW = 1;
		if (private_vblank->dpms != TDM_OUTPUT_DPMS_ON) {
#if 0
			if (private_vblank->SW_align_wait) {
				LIST_DEL(&private_vblank->SW_align_wait->valid_link);
				free(private_vblank->SW_align_wait);
				private_vblank->SW_align_wait = NULL;
			}
#endif

			if (private_vblank->enable_fake)
				_tdm_vblank_change_to_SW(private_vblank);
			else
				_tdm_vblank_free_HW_wait(private_vblank, TDM_ERROR_DPMS_OFF, 1);
		}
		break;
	case TDM_OUTPUT_CHANGE_CONNECTION:
		VIN("output %s", tdm_status_str(value.u32));
		if (value.u32 == TDM_OUTPUT_CONN_STATUS_DISCONNECTED)
			_tdm_vblank_free_HW_wait(private_vblank, 0, 0);
		break;
	default:
		break;
	}
}

tdm_vblank*
tdm_vblank_create(tdm_display *dpy, tdm_output *output, tdm_error *error)
{
	tdm_private_vblank *private_vblank;
	const tdm_output_mode *mode = NULL;
	tdm_output_dpms dpms = TDM_OUTPUT_DPMS_ON;
	tdm_error ret;

	TDM_RETURN_VAL_IF_FAIL_WITH_ERROR(dpy != NULL, TDM_ERROR_INVALID_PARAMETER, NULL);
	TDM_RETURN_VAL_IF_FAIL_WITH_ERROR(output != NULL, TDM_ERROR_INVALID_PARAMETER, NULL);

	if (error)
		*error = TDM_ERROR_NONE;

	if (!vblank_list_inited) {
		LIST_INITHEAD(&vblank_list);
		LIST_INITHEAD(&valid_wait_list);
		vblank_list_inited = 1;
	}

	tdm_output_get_mode(output, &mode);
	if (!mode) {
		if (error)
			*error = TDM_ERROR_OPERATION_FAILED;
		TDM_ERR("no mode");
		return NULL;
	}

	tdm_output_get_dpms(output, &dpms);

	private_vblank = calloc(1, sizeof *private_vblank);
	if (!private_vblank) {
		if (error)
			*error = TDM_ERROR_OUT_OF_MEMORY;
		VER("alloc failed");
		return NULL;
	}

	tdm_output_add_change_handler(output, _tdm_vblank_cb_output_change, private_vblank);

	private_vblank->dpy = dpy;
	private_vblank->output = output;
	private_vblank->dpms = dpms;
	private_vblank->vrefresh = mode->vrefresh;
	private_vblank->HW_vblank_gap = (double)1000000 / private_vblank->vrefresh;

	private_vblank->check_HW_or_SW = 1;
	private_vblank->fps = mode->vrefresh;

	LIST_INITHEAD(&private_vblank->HW_wait_list);

	LIST_INITHEAD(&private_vblank->SW_pending_wait_list);
	LIST_INITHEAD(&private_vblank->SW_wait_list);

	LIST_ADD(&private_vblank->link, &vblank_list);

	VDB("created. vrefresh(%d) dpms(%d)",
		private_vblank->vrefresh, private_vblank->dpms);

	return (tdm_vblank*)private_vblank;
}

void
tdm_vblank_destroy(tdm_vblank *vblank)
{
	tdm_private_vblank *private_vblank = vblank;
	tdm_vblank_wait_info *w = NULL, *ww = NULL;

	if (!private_vblank)
		return;

	LIST_DEL(&private_vblank->link);

#if 0
	if (private_vblank->SW_align_wait) {
		LIST_DEL(&private_vblank->SW_align_wait->valid_link);
		free(private_vblank->SW_align_wait);
	}
#endif

	if (private_vblank->SW_timer) {
		if (tdm_display_lock(private_vblank->dpy) == TDM_ERROR_NONE) {
			tdm_event_loop_source_remove(private_vblank->SW_timer);
			tdm_display_unlock(private_vblank->dpy);
		}
	}

	tdm_output_remove_change_handler(private_vblank->output,
									 _tdm_vblank_cb_output_change, private_vblank);

	_tdm_vblank_free_HW_wait(private_vblank, 0, 0);

	LIST_FOR_EACH_ENTRY_SAFE(w, ww, &private_vblank->SW_pending_wait_list, link) {
		LIST_DEL(&w->link);
		LIST_DEL(&w->valid_link);
		free(w);
	}

	LIST_FOR_EACH_ENTRY_SAFE(w, ww, &private_vblank->SW_wait_list, link) {
		LIST_DEL(&w->link);
		LIST_DEL(&w->valid_link);
		free(w);
	}

	VIN("destroyed");

	free(private_vblank);
}

tdm_error
tdm_vblank_set_fps(tdm_vblank *vblank, unsigned int fps)
{
	tdm_private_vblank *private_vblank = vblank;

	TDM_RETURN_VAL_IF_FAIL(private_vblank != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(fps > 0, TDM_ERROR_INVALID_PARAMETER);

	if (private_vblank->fps == fps)
		return TDM_ERROR_NONE;

	private_vblank->fps = fps;
	private_vblank->check_HW_or_SW = 1;

	VDB("fps(%d)", private_vblank->fps);

	return TDM_ERROR_NONE;
}

tdm_error
tdm_vblank_set_offset(tdm_vblank *vblank, int offset)
{
	tdm_private_vblank *private_vblank = vblank;

	TDM_RETURN_VAL_IF_FAIL(private_vblank != NULL, TDM_ERROR_INVALID_PARAMETER);

	if (private_vblank->offset == offset)
		return TDM_ERROR_NONE;

	private_vblank->offset = offset;
	private_vblank->check_HW_or_SW = 1;

	VDB("offset(%d)", private_vblank->offset);

	return TDM_ERROR_NONE;
}

tdm_error
tdm_vblank_set_enable_fake(tdm_vblank *vblank, unsigned int enable_fake)
{
	tdm_private_vblank *private_vblank = vblank;

	TDM_RETURN_VAL_IF_FAIL(private_vblank != NULL, TDM_ERROR_INVALID_PARAMETER);

	if (private_vblank->enable_fake == enable_fake)
		return TDM_ERROR_NONE;

	private_vblank->enable_fake = enable_fake;

	VDB("enable_fake(%d)", private_vblank->enable_fake);

	return TDM_ERROR_NONE;
}

static void
_tdm_vblank_cb_vblank_HW(tdm_output *output, unsigned int sequence,
						 unsigned int tv_sec, unsigned int tv_usec,
						 void *user_data)
{
	tdm_vblank_wait_info *wait_info = user_data;
	tdm_private_vblank *private_vblank;

	if (!_tdm_vblank_check_valid(wait_info)) {
		TDM_DBG("can't find wait(%p) from valid_wait_list", wait_info);
		return;
	}

	private_vblank = wait_info->private_vblank;
	TDM_RETURN_IF_FAIL(private_vblank != NULL);

	if (!_tdm_vblank_find_wait(wait_info, &private_vblank->HW_wait_list)) {
		VDB("can't find wait(%p)", wait_info);
		return;
	}

	LIST_DEL(&wait_info->link);
	LIST_DEL(&wait_info->valid_link);

	private_vblank->last_seq = wait_info->target_seq;
	private_vblank->last_tv_sec = tv_sec;
	private_vblank->last_tv_usec = tv_usec;

	if (wait_info->func)
		wait_info->func(private_vblank, TDM_ERROR_NONE, private_vblank->last_seq,
						tv_sec, tv_usec, wait_info->user_data);

	VDB("wait(%p) done", wait_info);

	free(wait_info);
}

static tdm_error
_tdm_vblank_wait_HW(tdm_vblank_wait_info *wait_info)
{
	tdm_private_vblank *private_vblank = wait_info->private_vblank;
	tdm_error ret;

	TDM_RETURN_VAL_IF_FAIL(wait_info->target_hw_interval > 0, TDM_ERROR_OPERATION_FAILED);

	_tdm_vblank_insert_wait(wait_info, &private_vblank->HW_wait_list);

	ret = tdm_output_wait_vblank(private_vblank->output, wait_info->target_hw_interval, 0,
								 _tdm_vblank_cb_vblank_HW, wait_info);

	if (ret != TDM_ERROR_NONE) {
		VER("wait(%p) failed", wait_info);
		LIST_DEL(&wait_info->link);
		return ret;
	}

	VDB("wait(%p) waiting", wait_info);

	return TDM_ERROR_NONE;
}

static tdm_error
_tdm_vblank_cb_vblank_SW(void *user_data)
{
	tdm_private_vblank *private_vblank = user_data;
	tdm_vblank_wait_info *first_wait_info = NULL, *w = NULL, *ww = NULL;

	TDM_RETURN_VAL_IF_FAIL(private_vblank != NULL, TDM_ERROR_OPERATION_FAILED);

	first_wait_info = container_of(private_vblank->SW_wait_list.next, first_wait_info, link);
	TDM_RETURN_VAL_IF_FAIL(first_wait_info != NULL, TDM_ERROR_OPERATION_FAILED);

	VDB("wait(%p) done", first_wait_info);

	private_vblank->last_seq = first_wait_info->target_seq;
	private_vblank->last_tv_sec = first_wait_info->target_sec;
	private_vblank->last_tv_usec = first_wait_info->target_usec;

	LIST_FOR_EACH_ENTRY_SAFE(w, ww, &private_vblank->SW_wait_list, link) {
		if (w->target_sec != first_wait_info->target_sec ||
			w->target_usec != first_wait_info->target_usec)
			break;

		LIST_DEL(&w->link);
		LIST_DEL(&w->valid_link);

		if (w->func)
			w->func(private_vblank, TDM_ERROR_NONE, w->target_seq,
					w->target_sec, w->target_usec,
					w->user_data);

		free(w);
	}

	return TDM_ERROR_NONE;
}

static void
_tdm_vblank_cb_vblank_SW_first(tdm_output *output, unsigned int sequence,
							   unsigned int tv_sec, unsigned int tv_usec,
							   void *user_data)
{
	tdm_vblank_wait_info *wait_info = user_data;
	tdm_private_vblank *private_vblank;
	tdm_vblank_wait_info *w = NULL, *ww = NULL;
	unsigned int min_interval = 0;
	unsigned long last;

	if (!_tdm_vblank_check_valid(wait_info))
		return;

	private_vblank = wait_info->private_vblank;
	TDM_RETURN_IF_FAIL(private_vblank != NULL);

	w = container_of((&private_vblank->SW_pending_wait_list)->next, w, link);
	TDM_RETURN_IF_FAIL(w != NULL);

	VDB("wait(%p) done", w);

	min_interval = w->interval;

	last = (unsigned long)tv_sec * 1000000 + tv_usec;
	last -= private_vblank->offset * 1000;

	private_vblank->last_seq = min_interval;
	private_vblank->last_tv_sec = last / 1000000;
	private_vblank->last_tv_usec = last % 1000000;

	LIST_FOR_EACH_ENTRY_SAFE(w, ww, &private_vblank->SW_pending_wait_list, link) {
		if (w->interval == min_interval) {
			LIST_DEL(&w->link);
			LIST_DEL(&w->valid_link);

			if (w->func)
				w->func(private_vblank, TDM_ERROR_NONE, private_vblank->last_seq,
						tv_sec, tv_usec, w->user_data);
			free(w);
		} else {
			LIST_DEL(&w->link);
			w->interval -= min_interval;
			_tdm_vblank_wait_SW(w);
		}
	}
}

static tdm_error
_tdm_vblank_sw_timer_update(tdm_private_vblank *private_vblank)
{
	tdm_vblank_wait_info *first_wait_info = NULL;
	unsigned long curr, target;
	int ms_delay;
	tdm_error ret;

	first_wait_info = container_of(private_vblank->SW_wait_list.next, first_wait_info, link);
	curr = tdm_helper_get_time_in_micros();
	target = first_wait_info->target_sec * 1000000 + first_wait_info->target_usec;

	if (target < curr)
		ms_delay = 1;
	else
		ms_delay = ceil((double)(target - curr) / 1000);

	if (ms_delay < 1)
		ms_delay = 1;

	VDB("wait(%p) curr(%4lu) target(%4lu) ms_delay(%d)",
		first_wait_info, curr, target, ms_delay);

	ret = tdm_display_lock(private_vblank->dpy);
	TDM_RETURN_VAL_IF_FAIL(ret == TDM_ERROR_NONE, ret);

	if (!private_vblank->SW_timer) {
		private_vblank->SW_timer =
			tdm_event_loop_add_timer_handler(private_vblank->dpy,
											 _tdm_vblank_cb_vblank_SW,
											 private_vblank,
											 &ret);
		if (!private_vblank->SW_timer) {
			tdm_display_unlock(private_vblank->dpy);
			VER("couldn't add timer");
			return ret;
		}
		VIN("Use SW vblank");
	}

	ret = tdm_event_loop_source_timer_update(private_vblank->SW_timer, ms_delay);
	if (ret != TDM_ERROR_NONE) {
		tdm_display_unlock(private_vblank->dpy);
		VER("couldn't update timer");
		return ret;
	}

	tdm_display_unlock(private_vblank->dpy);

	return TDM_ERROR_NONE;
}

#if 0
static void
_tdm_vblank_cb_vblank_align(tdm_output *output, unsigned int sequence,
							unsigned int tv_sec, unsigned int tv_usec,
							void *user_data)
{
	tdm_vblank_wait_info *align_info = user_data;
	tdm_private_vblank *private_vblank;
	unsigned int diff_sec, diff_usec;

	if (!_tdm_vblank_check_valid(align_info))
		return;

	private_vblank = align_info->private_vblank;
	TDM_RETURN_IF_FAIL(private_vblank != NULL);

	private_vblank->SW_align_wait = NULL;
	private_vblank->SW_align_sec = tv_sec;
	private_vblank->SW_align_usec = tv_usec;

	LIST_DEL(&align_info->valid_link);

	if (tv_usec > align_info->req_usec) {
		diff_usec = tv_usec - align_info->req_usec;
		diff_sec = tv_sec - align_info->req_sec;
	} else {
		diff_usec = 1000000 + tv_usec - align_info->req_usec;
		diff_sec = tv_sec - align_info->req_sec - 1;
	}

	private_vblank->SW_align_offset = (double)(1000000 - diff_sec * 1000000 - diff_usec) / private_vblank->vrefresh;

	free(align_info);

	/* align vblank continously only if non HW and DPMS on */
	if (!private_vblank->HW_enable && private_vblank->dpms == TDM_OUTPUT_DPMS_ON)
		_tdm_vblank_sw_timer_align(private_vblank);
}

static void
_tdm_vblank_sw_timer_align(tdm_private_vblank *private_vblank)
{
	tdm_vblank_wait_info *align_info;
	unsigned long curr;
	tdm_error ret;

	if (private_vblank->SW_align_wait)
		return;

	TDM_RETURN_IF_FAIL(private_vblank->dpms == TDM_OUTPUT_DPMS_ON);

	align_info = calloc(1, sizeof *align_info);
	if (!align_info) {
		VER("alloc failed");
		return;
	}

	LIST_ADDTAIL(&align_info->valid_link, &valid_wait_list);
	align_info->stamp = ++stamp;
	align_info->private_vblank = private_vblank;

	curr = tdm_helper_get_time_in_micros();
	align_info->req_sec = curr / 1000000;
	align_info->req_usec = curr % 1000000;

	ret = tdm_output_wait_vblank(private_vblank->output, private_vblank->vrefresh, 0,
								 _tdm_vblank_cb_vblank_align, align_info);
	if (ret != TDM_ERROR_NONE) {
		LIST_DEL(&align_info->valid_link);
		free(align_info);
		return;
	}

	private_vblank->SW_align_wait = align_info;
}
#endif

static tdm_error
_tdm_vblank_wait_SW(tdm_vblank_wait_info *wait_info)
{
	tdm_private_vblank *private_vblank = wait_info->private_vblank;
	tdm_error ret;

	if (private_vblank->last_tv_sec == 0 && private_vblank->dpms == TDM_OUTPUT_DPMS_ON) {
		unsigned int do_wait = LIST_IS_EMPTY(&private_vblank->SW_pending_wait_list);
		_tdm_vblank_insert_wait(wait_info, &private_vblank->SW_pending_wait_list);
		if (do_wait) {
			ret = tdm_output_wait_vblank(private_vblank->output, 1, 0,
										 _tdm_vblank_cb_vblank_SW_first, wait_info);
			if (ret != TDM_ERROR_NONE) {
				LIST_DEL(&wait_info->link);
				return ret;
			}
			VDB("wait(%p) waiting", wait_info);
		}
		return TDM_ERROR_NONE;
	}

	TDM_RETURN_VAL_IF_FAIL(wait_info->target_sec > 0, TDM_ERROR_OPERATION_FAILED);

	_tdm_vblank_insert_wait(wait_info, &private_vblank->SW_wait_list);

	ret = _tdm_vblank_sw_timer_update(private_vblank);
	if (ret != TDM_ERROR_NONE) {
		VER("couldn't update sw timer");
		return ret;
	}

	return TDM_ERROR_NONE;
}

static void
_tdm_vblank_calculate_target(tdm_vblank_wait_info *wait_info)
{
	tdm_private_vblank *private_vblank = wait_info->private_vblank;
	unsigned long last, prev, req, curr, target;
	unsigned int skip = 0;

	curr = tdm_helper_get_time_in_micros();

	if (!private_vblank->HW_enable) {
		if (private_vblank->last_tv_sec == 0) {
			/* If last == 0 and DPMS == on, we will use HW vblank to sync with HW vblank. */
			if (private_vblank->dpms == TDM_OUTPUT_DPMS_ON) {
				return;
			} else {
				private_vblank->last_tv_sec = curr / 1000000;
				private_vblank->last_tv_usec = curr % 1000000;
			}
		}
	}

	/* last can be 0 when HW enable. But it doesn't matter if HW enable. */
	if (!private_vblank->HW_enable)
		TDM_RETURN_IF_FAIL(private_vblank->last_tv_sec != 0);

	last = (unsigned long)private_vblank->last_tv_sec * 1000000 + private_vblank->last_tv_usec;
	req = (unsigned long)wait_info->req_sec * 1000000 + wait_info->req_usec;
	skip = (unsigned int)((req - last) / private_vblank->vblank_gap);
	prev = last + skip * private_vblank->vblank_gap;

	if (private_vblank->last_seq == 0)
		skip = 0;

	skip += wait_info->interval;

	if (private_vblank->HW_enable) {
		unsigned int hw_skip = (unsigned int)((curr - prev) / private_vblank->HW_vblank_gap);

		wait_info->target_hw_interval = wait_info->interval * private_vblank->HW_quotient;
		wait_info->target_hw_interval -= hw_skip;

		if (wait_info->target_hw_interval < 1)
			wait_info->target_hw_interval = 1;

		target = prev + wait_info->target_hw_interval * private_vblank->HW_vblank_gap;
	} else {
		target = prev + (unsigned long)(private_vblank->vblank_gap * wait_info->interval);

		while (target < curr) {
			target += (unsigned long)private_vblank->vblank_gap;
			skip++;
		}
	}

	VDB("target_seq(%d) last_seq(%d) skip(%d)",
		wait_info->target_seq, private_vblank->last_seq, skip);

#if 0
	target -= (private_vblank->SW_align_offset * skip * private_vblank->HW_quotient);
#endif

	wait_info->target_seq = private_vblank->last_seq + skip;
	wait_info->target_sec = target / 1000000;
	wait_info->target_usec = target % 1000000;

	VDB("wait(%p) last(%4lu) req(%4lu) prev(%4lu) curr(%4lu) skip(%d) hw_interval(%d) target(%4lu,%4lu)",
		wait_info, last, req - last, prev - last, curr - last,
		skip, wait_info->target_hw_interval, target, target - last);
}

tdm_error
tdm_vblank_wait(tdm_vblank *vblank, unsigned int req_sec, unsigned int req_usec,
				unsigned int interval, tdm_vblank_handler func, void *user_data)
{
	tdm_private_vblank *private_vblank = vblank;
	tdm_vblank_wait_info *wait_info;
	tdm_error ret;

	TDM_RETURN_VAL_IF_FAIL(private_vblank != NULL, TDM_ERROR_INVALID_PARAMETER);
	TDM_RETURN_VAL_IF_FAIL(func != NULL, TDM_ERROR_INVALID_PARAMETER);

	if (private_vblank->dpms != TDM_OUTPUT_DPMS_ON && !private_vblank->enable_fake) {
		VIN("can't wait a vblank because of DPMS off");
		return TDM_ERROR_DPMS_OFF;
	}

#if 0
	if (!private_vblank->SW_align_wait && private_vblank->dpms == TDM_OUTPUT_DPMS_ON)
		_tdm_vblank_sw_timer_align(private_vblank);
#endif

	if (private_vblank->check_HW_or_SW) {
		private_vblank->check_HW_or_SW = 0;
		private_vblank->vblank_gap = (double)1000000 / private_vblank->fps;
		private_vblank->HW_quotient = private_vblank->vrefresh / private_vblank->fps;

		if (private_vblank->dpms == TDM_OUTPUT_DPMS_ON &&
			!(private_vblank->vrefresh % private_vblank->fps)) {
			private_vblank->HW_enable = 1;
			VIN("Use HW vblank");
		} else {
			private_vblank->HW_enable = 0;
			VIN("Use SW vblank");
		}
	}

	wait_info = calloc(1, sizeof *wait_info);
	if (!wait_info) {
		VER("alloc failed");
		return TDM_ERROR_OUT_OF_MEMORY;
	}

	LIST_ADDTAIL(&wait_info->valid_link, &valid_wait_list);
	wait_info->stamp = ++stamp;
	wait_info->req_sec = req_sec;
	wait_info->req_usec = req_usec;
	wait_info->interval = interval;
	wait_info->func = func;
	wait_info->user_data = user_data;
	wait_info->private_vblank = private_vblank;

	_tdm_vblank_calculate_target(wait_info);

	if (private_vblank->HW_enable)
		ret = _tdm_vblank_wait_HW(wait_info);
	else
		ret = _tdm_vblank_wait_SW(wait_info);

	if (ret != TDM_ERROR_NONE) {
		LIST_DEL(&wait_info->valid_link);
		free(wait_info);
		return ret;
	}

	return TDM_ERROR_NONE;
}
