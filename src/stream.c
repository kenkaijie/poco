// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Implementation for streams.
 */

#include <poco/context.h>
#include <poco/coro.h>
#include <poco/intracoro.h>
#include <poco/stream.h>
#include <string.h>

static inline bool _is_power_of_two(size_t buffer_size) {
    /* Power of 2 bit trick. */
    return ((buffer_size & (buffer_size - 1)) == 0);
}

stream_t *stream_create_static(stream_t *stream, size_t buffer_size, uint8_t *buffer) {

    if ((buffer_size == 0) || !_is_power_of_two(buffer_size)) {
        /* not a power of 2. */
        return NULL;
    }

    stream->buffer = buffer;
    stream->max_size = buffer_size;
    stream->read_idx = 0;
    stream->write_idx = 0;

    return stream;
}

stream_t *stream_create(size_t buffer_size) {
    stream_t *stream = (stream_t *)malloc(sizeof(stream_t));

    if (stream == NULL) {
        /* No memory. */
        return NULL;
    }

    uint8_t *stream_buffer = (uint8_t *)malloc(buffer_size * sizeof(uint8_t));

    if (stream_buffer == NULL) {
        /* No memory for buffer. */
        free(stream);
        return NULL;
    }

    stream_t *stream_handle = stream_create_static(stream, buffer_size, stream_buffer);
    if (stream_handle == NULL) {
        free(stream);
        free(stream_buffer);
    }
    return stream_handle;
}

void stream_free(stream_t *stream) {
    if (stream == NULL) {
        /* Cannot free null pointer, need a non null to free the internal buffer. */
        return;
    }

    if (stream->buffer != NULL) {
        free(stream->buffer);
    }

    free(stream);
}

size_t stream_bytes_used(stream_t *stream) {
    return (stream->write_idx - stream->read_idx) % stream->max_size;
}

size_t stream_bytes_free(stream_t *stream) {
    return stream->max_size - stream_bytes_used(stream);
}

result_t stream_send(stream_t *stream, uint8_t const *data, size_t *data_size,
                     platform_ticks_t timeout) {

    coro_t *coro = context_get_coro();
    size_t bytes_remaining = *data_size;
    size_t bytes_written = 0;

    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type = CORO_EVTSINK_STREAM_NOT_FULL;
    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].params.subject = stream;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_DELAY;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].params.ticks_remaining = timeout;

    while (bytes_remaining > 0) {

        size_t bytes_available = stream_bytes_free(stream);

        // write as much as we can
        if (bytes_remaining < bytes_available) {
            bytes_available = bytes_remaining;
        }

        if (bytes_available == 0) {
            /* No bytes available, we block and wait. */
            coro_yield_with_signal(CORO_SIG_WAIT);
            if (coro->triggered_event_sink_slot == EVENT_SINK_SLOT_TIMEOUT) {
                /* Timeout. */
                break;
            }
        }

        for (size_t count = 0; count < bytes_available; ++count) {
            stream->buffer[stream->write_idx % stream->max_size] =
                data[bytes_written + count];
            stream->write_idx += 1;
        }
        bytes_written += bytes_available;
        bytes_remaining -= bytes_available;
    }

    if (bytes_written > 0) {
        /* Notify the consumer if we have put even a single byte. */
        coro->event_source.type = CORO_EVTSRC_STREAM_SEND;
        coro->event_source.params.subject = stream;
        coro_yield_with_signal(CORO_SIG_NOTIFY);
    }

    *data_size = bytes_written;

    return (bytes_remaining == 0) ? RES_OK : RES_TIMEOUT;
}

result_t stream_send_no_wait(stream_t *stream, uint8_t const *data, size_t *data_size) {

    result_t notify_result = RES_OK;
    scheduler_t *scheduler = context_get_scheduler();
    size_t bytes_written = stream_bytes_free(stream);

    // write as much as we can
    if (*data_size < bytes_written) {
        bytes_written = *data_size;
    }

    for (size_t count = 0; count < bytes_written; ++count) {
        stream->buffer[stream->write_idx % stream->max_size] = data[count];
        stream->write_idx += 1;
    }

    if (bytes_written > 0) {
        /* Notify the consumer if we have put even a single byte. */
        coro_event_source_t const event = {.type = CORO_EVTSRC_STREAM_SEND,
                                           .params.subject = stream};
        notify_result = scheduler_notify(scheduler, &event);
    }

    *data_size = bytes_written;

    if (notify_result != RES_OK) {
        /* Critical failure to notify scheduler. */
        return RES_NOTIFY_FAILED;
    }

    return (bytes_written > 0) ? RES_OK : RES_STREAM_FULL;
}

result_t stream_send_from_isr(stream_t *stream, uint8_t const *data,
                              size_t *data_size) {
    result_t notify_result = RES_OK;
    scheduler_t *scheduler = context_get_scheduler();
    size_t bytes_written = stream_bytes_free(stream);

    // write as much as we can
    if (*data_size < bytes_written) {
        bytes_written = *data_size;
    }

    for (size_t count = 0; count < bytes_written; ++count) {
        stream->buffer[stream->write_idx % stream->max_size] = data[count];
        stream->write_idx += 1;
    }

    if (bytes_written > 0) {
        /* Notify the consumer if we have put even a single byte. */
        coro_event_source_t const event = {.type = CORO_EVTSRC_STREAM_SEND,
                                           .params.subject = stream};
        notify_result = scheduler_notify_from_isr(scheduler, &event);
    }

    *data_size = bytes_written;

    if (notify_result != RES_OK) {
        /* Critical failure to notify scheduler. */
        return RES_NOTIFY_FAILED;
    }

    return (bytes_written > 0) ? RES_OK : RES_STREAM_FULL;
}

result_t stream_receive(stream_t *stream, uint8_t *buffer, size_t *buffer_size,
                        platform_ticks_t timeout) {

    coro_t *coro = context_get_coro();
    size_t bytes_remaining = *buffer_size;
    size_t bytes_read = 0;

    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type = CORO_EVTSINK_STREAM_NOT_EMPTY;
    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].params.subject = stream;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_DELAY;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].params.ticks_remaining = timeout;

    while (bytes_remaining > 0) {

        size_t bytes_available = stream_bytes_used(stream);

        if (bytes_remaining < bytes_available) {
            bytes_available = bytes_remaining;
        }

        if (bytes_available == 0) {
            /* No bytes available, we block and wait. */
            coro_yield_with_signal(CORO_SIG_WAIT);
            if (coro->triggered_event_sink_slot == EVENT_SINK_SLOT_TIMEOUT) {
                /* Timeout. */
                break;
            }
        }

        for (size_t count = 0; count < bytes_available; ++count) {
            buffer[bytes_read + count] =
                stream->buffer[stream->read_idx % stream->max_size];
            stream->read_idx += 1;
        }
        /* As we are reading, if they are the same, they must be empty. */
        bytes_read += bytes_available;
        bytes_remaining -= bytes_available;
    }

    if (bytes_read > 0) {
        /* Notify the producer if we have taken out any bytes. */
        coro->event_source.type = CORO_EVTSRC_STREAM_RECV;
        coro->event_source.params.subject = stream;
        coro_yield_with_signal(CORO_SIG_NOTIFY);
    }

    *buffer_size = bytes_read;

    return (bytes_remaining == 0) ? RES_OK : RES_TIMEOUT;
}

result_t stream_receive_up_to(stream_t *stream, uint8_t *buffer, size_t *buffer_size,
                              platform_ticks_t timeout) {

    /* This is quite similar to the standard receive, except without a loop. */
    coro_t *coro = context_get_coro();

    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type = CORO_EVTSINK_STREAM_NOT_EMPTY;
    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].params.subject = stream;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_DELAY;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].params.ticks_remaining = timeout;

    size_t bytes_available = stream_bytes_used(stream);

    if (bytes_available == 0) {
        /* No bytes available, we block and wait. */
        coro_yield_with_signal(CORO_SIG_WAIT);
        /* we just recheck regardless of timeout or actual data. */
        bytes_available = stream_bytes_used(stream);
    }

    if (*buffer_size < bytes_available) {
        bytes_available = *buffer_size;
    }

    for (size_t count = 0; count < bytes_available; ++count) {
        buffer[count] = stream->buffer[stream->read_idx % stream->max_size];
        stream->read_idx += 1;
    }

    if (bytes_available > 0) {
        /* Notify as we have read some data */
        coro->event_source.type = CORO_EVTSRC_STREAM_RECV;
        coro->event_source.params.subject = stream;
        coro_yield_with_signal(CORO_SIG_NOTIFY);
    }

    *buffer_size = bytes_available;

    return (bytes_available > 0) ? RES_OK : RES_TIMEOUT;
}

result_t stream_receive_no_wait(stream_t *stream, uint8_t *buffer,
                                size_t *buffer_size) {
    result_t notify_result = RES_OK;
    scheduler_t *scheduler = context_get_scheduler();
    size_t bytes_read = stream_bytes_used(stream);

    if (*buffer_size < bytes_read) {
        bytes_read = *buffer_size;
    }

    for (size_t count = 0; count < bytes_read; ++count) {
        buffer[count] = stream->buffer[stream->read_idx % stream->max_size];
        stream->read_idx += 1;
    }

    if (bytes_read > 0) {
        /* Notify the producer if we have taken out any bytes. */
        coro_event_source_t const event = {.type = CORO_EVTSRC_STREAM_RECV,
                                           .params.subject = stream};
        notify_result = scheduler_notify_from_isr(scheduler, &event);
    }

    *buffer_size = bytes_read;

    if (notify_result != RES_OK) {
        /* Critical failure to notify scheduler. */
        return RES_NOTIFY_FAILED;
    }

    return (bytes_read > 0) ? RES_OK : RES_STREAM_EMPTY;
}

result_t stream_receive_from_isr(stream_t *stream, uint8_t *buffer,
                                 size_t *buffer_size) {
    result_t notify_result = RES_OK;
    scheduler_t *scheduler = context_get_scheduler();
    size_t bytes_read = stream_bytes_used(stream);

    if (*buffer_size < bytes_read) {
        bytes_read = *buffer_size;
    }

    for (size_t count = 0; count < bytes_read; ++count) {
        buffer[count] = stream->buffer[stream->read_idx % stream->max_size];
        stream->read_idx += 1;
    }

    if (bytes_read > 0) {
        /* Notify the producer if we have taken out any bytes. */
        coro_event_source_t const event = {.type = CORO_EVTSRC_STREAM_RECV,
                                           .params.subject = stream};
        notify_result = scheduler_notify_from_isr(scheduler, &event);
    }

    *buffer_size = bytes_read;

    if (notify_result != RES_OK) {
        /* Critical failure to notify scheduler. */
        return RES_NOTIFY_FAILED;
    }

    return (bytes_read > 0) ? RES_OK : RES_STREAM_EMPTY;
}

result_t stream_flush(stream_t *stream, platform_ticks_t timeout) {
    coro_t *coro = context_get_coro();

    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type = CORO_EVTSINK_STREAM_NOT_FULL;
    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].params.subject = stream;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_DELAY;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].params.ticks_remaining = timeout;

    size_t bytes_remaining = stream_bytes_used(stream);

    while (bytes_remaining > 0) {

        bytes_remaining = stream_bytes_used(stream);

        if (bytes_remaining > 0) {
            /* No bytes available, we block and wait. */
            coro_yield_with_signal(CORO_SIG_WAIT);
            if (coro->triggered_event_sink_slot == EVENT_SINK_SLOT_TIMEOUT) {
                /* Timeout. */
                break;
            }
        }
    }

    return (bytes_remaining == 0) ? RES_OK : RES_TIMEOUT;
}