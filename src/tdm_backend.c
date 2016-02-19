/**************************************************************************

libtdm

Copyright 2015 Samsung Electronics co., Ltd. All Rights Reserved.

Contact: Eunchul Kim <chulspro.kim@samsung.com>,
         JinYoung Jeon <jy0.jeon@samsung.com>,
         Taeheon Kim <th908.kim@samsung.com>,
         YoungJun Cho <yj44.cho@samsung.com>,
         SooChan Lim <sc1.lim@samsung.com>,
         Boram Park <sc1.lim@samsung.com>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tdm.h"
#include "tdm_backend.h"
#include "tdm_private.h"

#define BACKEND_FUNC_ENTRY() \
    tdm_private_display *private_display; \
    TDM_RETURN_VAL_IF_FAIL(dpy != NULL, TDM_ERROR_INVALID_PARAMETER); \
    private_display = (tdm_private_display*)dpy;

EXTERN tdm_error
tdm_backend_register_func_display(tdm_display *dpy,
                                  tdm_func_display *func_display)
{
	BACKEND_FUNC_ENTRY();

	TDM_RETURN_VAL_IF_FAIL(func_display != NULL, TDM_ERROR_INVALID_PARAMETER);

	pthread_mutex_lock(&private_display->lock);
	private_display->func_display = *func_display;
	pthread_mutex_unlock(&private_display->lock);

	return TDM_ERROR_NONE;
}

EXTERN tdm_error
tdm_backend_register_func_pp(tdm_display *dpy, tdm_func_pp *func_pp)
{
	BACKEND_FUNC_ENTRY();

	if (!func_pp)
		return TDM_ERROR_NONE;

	pthread_mutex_lock(&private_display->lock);
	private_display->capabilities |= TDM_DISPLAY_CAPABILITY_PP;
	private_display->func_pp = *func_pp;
	pthread_mutex_unlock(&private_display->lock);

	return TDM_ERROR_NONE;
}

EXTERN tdm_error
tdm_backend_register_func_capture(tdm_display *dpy,
                                  tdm_func_capture *func_capture)
{
	BACKEND_FUNC_ENTRY();

	if (!func_capture)
		return TDM_ERROR_NONE;

	pthread_mutex_lock(&private_display->lock);
	private_display->capabilities |= TDM_DISPLAY_CAPABILITY_CAPTURE;
	private_display->func_capture = *func_capture;
	pthread_mutex_unlock(&private_display->lock);

	return TDM_ERROR_NONE;
}

