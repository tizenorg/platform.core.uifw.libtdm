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

#ifndef _TDM_CLIENT_H_
#define _TDM_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file tdm_client.h
 * @brief The header file for a client of TDM.
 * @par Example
 * @code
 * #include <tdm_client.h>    //for a client of TDM
 * @endcode
 */

#include <tdm_client_types.h>

/**
 * @brief Create a TDM client object.
 * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
 * @return A TDM client object if success. Otherwise, NULL.
 * @see #tdm_client_destroy
 */
tdm_client*
tdm_client_create(tdm_error *error);

/**
 * @brief Destroy a TDM client object
 * @param[in] client A TDM client object
 * @see #tdm_client_create
 */
void
tdm_client_destroy(tdm_client *client);

/**
 * @brief Get the file descriptor
 * @param[in] client A TDM client object
 * @param[out] fd The file descriptor
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see #tdm_client_handle_events
 * @par Example
 * @code
 * #include <tdm_client.h>    //for a client of TDM
 *
 * err = tdm_client_get_fd(client, &fd);
 * if (err != TDM_ERROR_NONE) {
 *     //error handling
 * }
 *
 * fds.events = POLLIN;
 * fds.fd = fd;
 * fds.revents = 0;
 *
 * while(1) {
 *    ret = poll(&fds, 1, -1);
 *    if (ret < 0) {
 *       if (errno == EINTR || errno == EAGAIN)
 *          continue;
 *       else {
 *          //error handling
 *       }
 *    }
 *
 *    err = tdm_client_handle_events(client);
 *    if (err != TDM_ERROR_NONE) {
 *        //error handling
 *    }
 * }
 * @endcode
 */
tdm_error
tdm_client_get_fd(tdm_client *client, int *fd);

/**
 * @brief Handle the events of the given file descriptor
 * @param[in] client A TDM client object
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see #tdm_client_get_fd
 */
tdm_error
tdm_client_handle_events(tdm_client *client);

/**
 * @brief @b Deprecated. Wait for VBLANK.
 * @deprecated
 * @details After interval vblanks, a client vblank handler will be called.
 * If 'sw_timer' param is 1 in case of DPMS off, TDM will use the SW timer and
 * call a client vblank handler. Otherwise, this function will return error.
 * @param[in] client A TDM client object
 * @param[in] name The name of a TDM output
 * @param[in] sw_timer 0: not using SW timer, 1: using SW timer
 * @param[in] interval vblank interval
 * @param[in] sync 0: asynchronous, 1:synchronous
 * @param[in] func A client vblank handler
 * @param[in] user_data The user data
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @see #tdm_client_vblank_handler
 */
tdm_error
tdm_client_wait_vblank(tdm_client *client, char *name,
					   int sw_timer, int interval, int sync,
					   tdm_client_vblank_handler2 func, void *user_data);

/**
 * @brief Get the client output object which has the given name.
 * @details
 * The client output name can be @b 'primary' or @b 'default' to get the main output.
 * @param[in] client The TDM client object
 * @param[in] name The name of the TDM client output object
 * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
 * @return A client output object if success. Otherwise, NULL.
 */
tdm_client_output*
tdm_client_get_output(tdm_client *client, char *name, tdm_error *error);

/**
 * @brief Add the client output change handler
 * @details The handler will be called when the status of a
 * client output object is changed. connection, DPMS, etc.
 * @param[in] output The client output object
 * @param[in] func The client output change handler
 * @param[in] user_data The user data
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @par Example
 * @code
 * #include <tdm_client.h>    //for a client of TDM
 *
 * static void
 * _client_output_handler(tdm_client_output *output, tdm_output_change_type type,
 *                        tdm_value value, void *user_data)
 * {
 *     char *conn_str[3] = {"disconnected", "connected", "mode_setted"};
 *     char *dpms_str[4] = {"on", "standy", "suspend", "off"};
 *
 *     if (type == TDM_OUTPUT_CHANGE_CONNECTION)
 *         printf("output %s.\n", conn_str[value.u32]);
 *     else if (type == TDM_OUTPUT_CHANGE_DPMS)
 *         printf("dpms %s.\n", dpms_str[value.u32]);
 * }
 * ...
 * tdm_client_output_add_change_handler(output, _client_output_handler, NULL);
 * ...
 * tdm_client_output_remove_change_handler(output, _client_output_handler, NULL);
 *
 * @endcode
 */
tdm_error
tdm_client_output_add_change_handler(tdm_client_output *output,
									 tdm_client_output_change_handler func,
									 void *user_data);

/**
 * @brief Remove the client output change handler
 * @param[in] output The client output object
 * @param[in] func The client output change handler
 * @param[in] user_data The user data
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
void
tdm_client_output_remove_change_handler(tdm_client_output *output,
										tdm_client_output_change_handler func,
										void *user_data);

/**
 * @brief Get the vertical refresh rate of the given client output
 * @param[in] output The client output object
 * @param[out] refresh The refresh rate
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_client_output_get_refresh_rate(tdm_client_output *output, unsigned int *refresh);

/**
 * @brief Get the connection status of the given client output
 * @param[in] output The client output object
 * @param[out] status The connection status
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_client_output_get_conn_status(tdm_client_output *output, tdm_output_conn_status *status);

/**
 * @brief Get the DPMS value of the given client output
 * @param[in] output The client output object
 * @param[out] dpms The DPMS value
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_client_output_get_dpms(tdm_client_output *output, tdm_output_dpms *dpms);

/**
 * @brief Create the client vblank object of the given client output
 * @param[in] output The client output object
 * @param[out] error #TDM_ERROR_NONE if success. Otherwise, error value.
 * @return A client vblank object if success. Otherwise, NULL.
 */
tdm_client_vblank*
tdm_client_output_create_vblank(tdm_client_output *output, tdm_error *error);

/**
 * @brief Destroy the client vblank object
 * @param[in] vblank The client vblank object
 */
void
tdm_client_vblank_destroy(tdm_client_vblank *vblank);

/**
 * @brief Set the sync value to the client vblank object
 * @details
 * If sync == 1, the user client vblank handler of #tdm_client_vblank_wait
 * will be called before #tdm_client_vblank_wait returns the result. If sync == 0,
 * the user client vblank handler of #tdm_client_vblank_wait will be called
 * asynchronously after #tdm_client_vblank_wait returns. Default is @b asynchronous.
 * @param[in] vblank The client vblank object
 * @param[in] sync 0: asynchronous, 1:synchronous
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_client_vblank_set_sync(tdm_client_vblank *vblank, unsigned int sync);

/**
 * @brief Set the fps to the client vblank object
 * @details Default is the @b vertical @b refresh @b rate of the given client output.
 * @param[in] vblank The client vblank object
 * @param[in] fps more than 0
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_client_vblank_set_fps(tdm_client_vblank *vblank, unsigned int fps);

/**
 * @brief Set the offset(milli-second) to the client vblank object
 * @details Default is @b 0.
 * @param[in] vblank The client vblank object
 * @param[in] offset_ms the offset(milli-second)
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_client_vblank_set_offset(tdm_client_vblank *vblank, int offset_ms);

/**
 * @brief Enable/Disable the fake vblank to the client vblank object
 * @details
 * If enable_fake == 0, #tdm_client_vblank_wait will return TDM_ERROR_DPMS_OFF
 * when DPMS off. Otherwise, #tdm_client_vblank_wait will return TDM_ERROR_NONE
 * as success. Once #tdm_client_vblank_wait returns TDM_ERROR_NONE, the user client
 * vblank handler(#tdm_client_vblank_handler) SHOULD be called after the given
 * interval of #tdm_client_vblank_wait. Default is @b disable.
 * @param[in] vblank The client vblank object
 * @param[in] enable_fake 1:enable, 0:disable
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 */
tdm_error
tdm_client_vblank_set_enable_fake(tdm_client_vblank *vblank, unsigned int enable_fake);

/**
 * @brief Wait for a vblank
 * @details
 * This function will return TDM_ERROR_DPMS_OFF when DPMS off. However,
 * #tdm_client_vblank_wait will return TDM_ERROR_NONE as success if
 * #tdm_client_vblank_set_enable_fake sets true. Once #tdm_client_vblank_wait
 * returns TDM_ERROR_NONE, the user client vblank handler(#tdm_client_vblank_handler)
 * SHOULD be called after the given interval.
 * @param[in] vblank The client vblank object
 * @param[in] interval The vblank interval
 * @param[in] func The user client vblank handler
 * @param[in] user_data The user data
 * @return #TDM_ERROR_NONE if success. Otherwise, error value.
 * @par Example
 * @code
 * #include <tdm_client.h>    //for a client of TDM
 *
 * static void
 * _client_vblank_handler(tdm_client_vblank *vblank, tdm_error error, unsigned int sequence,
 *                        unsigned int tv_sec, unsigned int tv_usec, void *user_data)
 * {
 *     if (error != TDM_ERROR_NONE)
 *         //error handling
 * }
 *
 * {
 *     tdm_client_output *output;
 *     tdm_client_vblank *vblank;
 *     tdm_error error;
 *     struct pollfd fds;
 *     int fd = -1;
 *
 *     cliet = tdm_client_create(&error);
 *     if (error != TDM_ERROR_NONE)
 *         //error handling
 *
 *     output = tdm_client_get_output(client, NULL, &error);
 *     if (error != TDM_ERROR_NONE)
 *         //error handling
 *
 *     vblank = tdm_client_output_create_vblank(output, &error);
 *     if (error != TDM_ERROR_NONE)
 *         //error handling
 *
 *     tdm_client_vblank_set_enable_fake(vblank, enable_fake); //default: disable
 *     tdm_client_vblank_set_sync(vblank, 0); //default: async
 *     tdm_client_vblank_set_fps(vblank, fps); //default: refresh rate of output
 *     tdm_client_vblank_set_offset(vblank, offset); //default: 0
 *
 *     error = tdm_client_get_fd(data->client, &fd);
 *     if (error != TDM_ERROR_NONE)
 *         //error handling
 *
 *     fds.events = POLLIN;
 *     fds.fd = fd;
 *     fds.revents = 0;
 *
 *     while (1) {
 *         int ret;
 *
 *         error = tdm_client_vblank_wait(vblank, interval,
 *                                        _client_vblank_handler, NULL);
 *         if (error != TDM_ERROR_NONE)
 *             //error handling
 *
 *         ret = poll(&fds, 1, -1);
 *         if (ret < 0) {
 *             if (errno == EINTR || errno == EAGAIN)  // normal case
 *                 continue;
 *             else
 *                 //error handling
 *         }
 *
 *         error = tdm_client_handle_events(client);
 *         if (error != TDM_ERROR_NONE)
 *             //error handling
 *     }
 *
 *     tdm_client_vblank_destroy(vblank);
 *     tdm_client_destroy(client);
 * }
 * @endcode
 */
tdm_error
tdm_client_vblank_wait(tdm_client_vblank *vblank, unsigned int interval, tdm_client_vblank_handler func, void *user_data);


#ifdef __cplusplus
}
#endif

#endif /* _TDM_CLIENT_H_ */
