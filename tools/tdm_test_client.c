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

#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <errno.h>
#include <time.h>

#include <tdm_client.h>
#include <tdm_helper.h>

static int
get_time_in_millis(void)
{
	struct timespec tp;

	if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
		return (int)(tp.tv_sec * 1000) + (tp.tv_nsec / 1000000L);

	return 0;
}

static void
_client_vblank_handler(unsigned int sequence, unsigned int tv_sec,
                       unsigned int tv_usec, void *user_data)
{
	int temp1, temp2;

	temp1 = (int)user_data;
	temp2 = get_time_in_millis();

	printf("%d ms\n", temp2 - temp1);
}


int
main(int argc, char *argv[])
{
	tdm_client *client;
	tdm_client_error error;
	int fd = -1;
	struct pollfd fds;

	client = tdm_client_create(&error);
	if (error != TDM_CLIENT_ERROR_NONE) {
		printf("tdm_client_create failed\n");
		exit(1);
	}

	error = tdm_client_get_fd(client, &fd);
	if (error != TDM_CLIENT_ERROR_NONE || fd < 0) {
		printf("tdm_client_get_fd failed\n");
		goto done;
	}

	fds.events = POLLIN;
	fds.fd = fd;
	fds.revents = 0;

	while (1) {
		int ret;

		error = tdm_client_wait_vblank(client, "unknown-0", 1, 0,
		                               _client_vblank_handler,
		                               (void*)get_time_in_millis());
		if (error != TDM_CLIENT_ERROR_NONE) {
			printf("tdm_client_wait_vblank failed\n");
			goto done;
		}

		ret = poll(&fds, 1, -1);
		if (ret < 0) {
			if (errno == EBUSY)  /* normal case */
				continue;
			else {
				printf("poll failed: %m\n");
				goto done;
			}
		}

		error = tdm_client_handle_events(client);
		if (error != TDM_CLIENT_ERROR_NONE)
			printf("tdm_client_handle_events failed\n");
	}

done:
	tdm_client_destroy(client);
	return 0;
}
