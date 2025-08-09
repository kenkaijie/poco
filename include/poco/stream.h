// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Streams are spsc queues specialised for byte data.
 *
 * The typical use case is for queuing bytes between ISRs and coroutines.
 *
 * The difference for this API and the queueing API is that streams treat the buffer
 * as a stream of bytes, whereas the queues treat the buffer as a set of elements.
 *
 * The API here will resemble typical stream based APIs (send, recv, flush).
 *
 * @note Some of these functions should only be called by the consumer, and some should
 * only be called by the producer.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <poco/platform.h>
#include <poco/result.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * @brief Queue specific result codes.
 */
enum res_code_stream {
    /*! The stream is empty. */
    RES_STREAM_EMPTY = RES_CODE(RES_GROUP_STREAM, 0),

    /*! The stream is full. */
    RES_STREAM_FULL = RES_CODE(RES_GROUP_STREAM, 1),
};

typedef struct stream {
    uint8_t *buffer;
    size_t max_size;
    size_t read_idx;
    size_t write_idx;
    bool buffer_is_empty; /**< We use this method of determining between FULL and Empty
                           */
} stream_t;

stream_t *stream_create_static(stream_t *stream, size_t buffer_size, uint8_t *buffer);

stream_t *stream_create(size_t buffer_size);

void stream_free(stream_t *stream);

/*!
 * @brief Sends data across the stream.
 *
 * @param stream Stream to send on.
 * @param data bytes to send
 * @param data_size the amount of data to be sent, in bytes. On return, displays the
 *      actual number of bytes sent.
 * @param timeout maximum amount of time to wait.
 *
 * @retval #RES_OK if the data has been placed into the stream.
 * @retval #RES_TIMEOUT if the timeout has elapsed without all data being sent. The
 *      value of data_size indicates the actual number of bytes that was sent.
 */
result_t stream_send(stream_t *stream, uint8_t const *data, size_t *data_size,
                     platform_ticks_t timeout);

/*!
 * @brief Receive a number of bytes from the stream.
 *
 * @param stream Stream to read from.
 * @param buffer Buffer to store the bytes
 * @param buffer_size Number of bytes to receive. On return, displays the actual number
 *      of bytes read.
 * @param timeout maximum amount of time to wait.
 *
 * @retval #RES_OK if the requested number of bytes has been read.
 * @retval #RES_TIMEOUT if the timeout has elapsed without all data being recevied. The
 *      value of buffer_size indicates the actual number of bytes read.
 */
result_t stream_recv(stream_t *stream, uint8_t *buffer, size_t *buffer_size,
                     platform_ticks_t timeout);

/*!
 * @brief Block the producer until the stream is completely empty.
 *
 * @param stream Stream to flush
 * @param timeout Maximum ticks to wait.
 *
 * @retval #RES_OK if the stream was flushed.
 * @retval #RES_TIMEOUT if the timeout elapsed without the stream being flushed.
 */
result_t stream_flush(stream_t *stream, platform_ticks_t timeout);

#ifdef __cplusplus
}
#endif
