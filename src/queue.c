/*!
 * @file
 * @brief Queue implementation for arbitray sized queues.
 *
 * SPDX-FileCopyrightText: Copyright contributors to the poco project.
 * SPDX-License-Identifier: MIT
 */

#include <poco/context.h>
#include <poco/coro.h>
#include <poco/intracoro.h>
#include <poco/queue.h>
#include <string.h>

/*!
 * @brief Unsafe push, does not perform checking and is for internal use only.
 */
__attribute__((always_inline)) static inline void _put(queue_t *queue,
                                                       void const *item) {
    memcpy(&queue->item_buffer[queue->write_idx * queue->item_size], item,
           queue->item_size);
    queue->write_idx = (queue->write_idx + 1) % queue->max_items;
    queue->count++;
}

/*!
 * @brief Unsafe pop, does not perform checking and is for internal use only.
 */
__attribute__((always_inline)) static inline void _get(queue_t *queue, void *item) {
    memcpy(item, &queue->item_buffer[queue->read_idx * queue->item_size],
           queue->item_size);
    queue->read_idx = (queue->read_idx + 1) % queue->max_items;
    queue->count--;
}

__attribute__((always_inline)) static inline bool _is_full(queue_t *queue) {
    return queue->count == queue->max_items;
}

__attribute__((always_inline)) static inline bool _is_empty(queue_t *queue) {
    return queue->count == 0;
}

__attribute__((always_inline)) static inline size_t _item_count(queue_t *queue) {
    return queue->count;
}

queue_t *queue_create_static(queue_t *queue, size_t num_items, size_t item_size,
                             uint8_t *item_buffer) {
    queue->item_buffer = item_buffer;
    queue->count = 0;
    queue->read_idx = 0;
    queue->write_idx = 0;
    queue->item_size = item_size;
    queue->max_items = num_items;
    return queue;
}

queue_t *queue_create(size_t num_items, size_t item_size) {
    queue_t *queue = (queue_t *)malloc(sizeof(queue_t));
    if (queue == NULL) {
        /* No memory. */
        return NULL;
    }
    uint8_t *item_buffer = (uint8_t *)malloc(num_items * item_size);
    if (item_buffer == NULL) {
        /* No memory. */
        free(queue);
        return NULL;
    }

    queue_t *queue_handle =
        queue_create_static(queue, num_items, item_size, item_buffer);
    if (queue_handle == NULL) {
        queue_free(queue);
    }
    return queue_handle;
}

void queue_free(queue_t *queue) {
    if (queue == NULL) {
        /* Cannot free null pointer, need a non null to free the internal buffer. */
        return;
    }

    if (queue->item_buffer != NULL) {
        free(queue->item_buffer);
    }

    free(queue);
}

size_t queue_item_count(queue_t *queue) {
    platform_enter_critical_section();
    size_t item_count = _item_count(queue);
    platform_exit_critical_section();
    return item_count;
}

bool queue_is_full(queue_t *queue) {
    platform_enter_critical_section();
    bool is_full = _is_full(queue);
    platform_exit_critical_section();
    return is_full;
}

bool queue_is_empty(queue_t *queue) {
    platform_enter_critical_section();
    bool is_empty = _is_empty(queue);
    platform_exit_critical_section();
    return is_empty;
}

result_t queue_raw_put(queue_t *queue, void const *item) {
    if (queue_is_full(queue)) {
        return RES_QUEUE_FULL;
    }

    _put(queue, item);
    return RES_OK;
}

result_t queue_raw_get(queue_t *queue, void *item) {
    if (queue_is_empty(queue)) {
        return RES_QUEUE_EMPTY;
    }

    _get(queue, item);

    return RES_OK;
}

result_t queue_put(queue_t *queue, void const *item, platform_ticks_t timeout) {

    coro_t *coro = context_get_coro();
    bool put_success = false;

    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type = CORO_EVTSINK_QUEUE_NOT_FULL;
    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].params.subject = queue;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_DELAY;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].params.ticks_remaining = timeout;

    while (!put_success) {

        platform_enter_critical_section();
        if (!_is_full(queue)) {
            _put(queue, item);
            put_success = true;
        }
        platform_exit_critical_section();

        if (!put_success) {

            coro_yield_with_signal(CORO_SIG_WAIT);

            if (coro->triggered_event_sink_slot == EVENT_SINK_SLOT_TIMEOUT) {
                /* Timeout. */
                break;
            }
        }
    }

    if (put_success) {
        coro->event_source.type = CORO_EVTSRC_QUEUE_PUT;
        coro->event_source.params.subject = queue;
        coro_yield_with_signal(CORO_SIG_NOTIFY);
    }

    return (put_success) ? RES_OK : RES_TIMEOUT;
}

result_t queue_get(queue_t *queue, void *item, platform_ticks_t timeout) {
    coro_t *coro = context_get_coro();
    bool get_success = false;

    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type = CORO_EVTSINK_QUEUE_NOT_EMPTY;
    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].params.subject = queue;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_DELAY;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].params.ticks_remaining = timeout;

    while (!get_success) {

        platform_enter_critical_section();
        if (!_is_empty(queue)) {
            _get(queue, item);
            get_success = true;
        }
        platform_exit_critical_section();

        if (!get_success) {
            coro_yield_with_signal(CORO_SIG_WAIT);

            if (coro->triggered_event_sink_slot == EVENT_SINK_SLOT_TIMEOUT) {
                /* Timeout. */
                break;
            }
        }
    }

    if (get_success) {
        coro->event_source.type = CORO_EVTSRC_QUEUE_GET;
        coro->event_source.params.subject = queue;
        coro_yield_with_signal(CORO_SIG_NOTIFY);
    }

    return (get_success) ? RES_OK : RES_TIMEOUT;
}

result_t queue_put_no_wait(queue_t *queue, void const *item) {
    coro_t *coro = context_get_coro();
    bool put_success = false;

    platform_enter_critical_section();
    if (!_is_full(queue)) {
        _put(queue, item);
        put_success = true;
    }
    platform_exit_critical_section();

    if (put_success) {
        coro->event_source.type = CORO_EVTSRC_QUEUE_PUT;
        coro->event_source.params.subject = queue;
        coro_yield_with_signal(CORO_SIG_NOTIFY);
    }

    return (put_success) ? RES_OK : RES_QUEUE_FULL;
}

result_t queue_get_no_wait(queue_t *queue, void *item) {
    coro_t *coro = context_get_coro();
    bool get_success = false;

    platform_enter_critical_section();
    if (!_is_empty(queue)) {
        _get(queue, item);
        get_success = true;
    }
    platform_exit_critical_section();

    if (get_success) {
        coro->event_source.type = CORO_EVTSRC_QUEUE_GET;
        coro->event_source.params.subject = queue;
        coro_yield_with_signal(CORO_SIG_NOTIFY);
    }

    return (get_success) ? RES_OK : RES_QUEUE_EMPTY;
}
