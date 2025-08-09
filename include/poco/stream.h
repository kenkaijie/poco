// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Streams are spsc queues specialised for byte data.
 *
 * The typical use case is for queuing bytes between ISRs and coroutines.
 *
 * To allow both the producer and consumer to "run" unbounded, there is a limitation
 * that the buffer size must be a multiple of 2.
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
} stream_t;

/*!
 * @brief Creates a static stream from
 *
 * @param stream Pointer to the statically allocated stream structure.
 * @param buffer_size Number of bytes in the stream. Must be a power of 2.
 * @param buffer a buffer the stream can use. Must be at least buffer_size bytes.
 *
 * @return a pointer to the stream (same as the input) or NULL if the stream could not
 *      be created.
 */
stream_t *stream_create_static(stream_t *stream, size_t buffer_size, uint8_t *buffer);

/*!
 * @brief Dynamically allocate space for a stream.
 *
 * @param buffer_size Number of bytes for the stream to use. Must be a power of 2.
 *
 * @return a pointer to a stream, or NULL if the stream could not be allocated.
 */
stream_t *stream_create(size_t buffer_size);

/*!
 * @brief Frees a dyanicamlly allocated stream.
 *
 * @param stream Stream to deallocate.
 */
void stream_free(stream_t *stream);

/*!
 * @brief Gets the number of bytes used in the stream.
 *
 * @warning This value only represents a snpashot. However:
 *
 *      - If called from the consumer, guaranteed to be greater than or equal.
 *
 *      - If called from the producer, guaranteed to be lesser than or equal.
 *
 * @param stream Stream to check.
 *
 * @return The number of bytes the stream has in use. See warning for more information.
 */
size_t stream_bytes_used(stream_t *stream);

/*!
 * @brief Gets the number of bytes used in the stream.
 *
 * @warning This value only represents a snpashot. However:
 *
 *      - If called from the consumer, guaranteed to be lesser than or equal.
 *
 *      - If called from the producer, guaranteed to be greater than or equal.
 *
 * @param stream Stream to check.
 *
 * @return The number of bytes the stream has for use. See warning for more information.
 */
size_t stream_bytes_free(stream_t *stream);

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
 * @brief Sends as much data onto the stream as possible without blocking.
 *
 * @param stream Stream to send.
 * @param data Data to send.
 * @param data_size Size of data, in bytes.
 *
 * @retval #RES_OK If data has been sent. Check the value of data_size to determine how
 *      muych was actually sent.
 * @retval #RES_STREAM_FULL If no bytes were sent.
 * @retval #RES_NOTIFY_FAILED if the scheduler notification has failed.
 */
result_t stream_send_no_wait(stream_t *stream, uint8_t const *data, size_t *data_size);

/*!
 * @brief Sends as much data onto the stream as possible without blocking from an ISR.
 *
 * @param stream Stream to send.
 * @param data Data to send.
 * @param data_size Size of data, in bytes.
 *
 * @retval #RES_OK If data has been sent. Check the value of data_size to determine how
 *      muych was actually sent.
 * @retval #RES_STREAM_FULL If no bytes were sent.
 */
result_t stream_send_from_isr(stream_t *stream, uint8_t const *data, size_t *data_size);

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
result_t stream_receive(stream_t *stream, uint8_t *buffer, size_t *buffer_size,
                        platform_ticks_t timeout);

/*!
 * @brief Receive up to a number of bytes from the stream.
 *
 * This differs from @ref stream_receive as it will not attempt to wait for the
 * requested length. This will only block if there is no data in the stream, and when
 * data is present, will simply return all there is.
 *
 * @param stream Stream to read from.
 * @param buffer Buffer to store the bytes
 * @param buffer_size Number of bytes to receive. On return, displays the actual number
 *      of bytes read.
 * @param timeout Maximum amount of time to wait.
 *
 * @retval #RES_OK if the requested number of bytes has been read.
 * @retval #RES_TIMEOUT if the timeout has elapsed without all data being recevied. The
 *      value of buffer_size indicates the actual number of bytes read.
 */
result_t stream_receive_up_to(stream_t *stream, uint8_t *buffer, size_t *buffer_size,
                              platform_ticks_t timeout);

/*!
 * @brief Receive a number of bytes without blocking.
 *
 * @param stream Stream to read.
 * @param buffer Buffer to read into.
 * @param buffer_size Maximum number of bytes to read.
 *
 * @retval #RES_OK if data has been received. Number of bytes read may be less than
 *      requested.
 * @retval #RES_STREAM_EMPTY if the stream was empty.
 * @retval #RES_NOTIFY_FAILED if the scheduler notification has failed.
 */
result_t stream_receive_no_wait(stream_t *stream, uint8_t *buffer, size_t *buffer_size);

/*!
 * @brief Receive a number of bytes from the ISR.
 *
 * @param stream Stream to read.
 * @param buffer Buffer to read into.
 * @param buffer_size Maximum number of bytes to read.
 *
 * @retval #RES_OK if data has been received.
 * @retval #RES_NOTIFY_FAILED if the scheduler notification has failed.
 */
result_t stream_receive_from_isr(stream_t *stream, uint8_t *buffer,
                                 size_t *buffer_size);

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
