// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Queue implementation for arbitray sized queues.
 */

#include <poco/context.h>
#include <poco/coro.h>
#include <poco/intracoro.h>
#include <poco/queue.h>
#include <poco/queue_raw.h>
#include <poco/scheduler.h>
#include <string.h>

/*!
 * @brief Unsafe push, does not perform checking and is for internal use only.
 */
static void _put(Queue *queue, void const *item) {
    memcpy(&queue->item_buffer[queue->write_idx * queue->item_size], item,
           queue->item_size);
    queue->write_idx = (queue->write_idx + 1) % queue->max_items;
    queue->count++;
}

/*!
 * @brief Unsafe pop, does not perform checking and is for internal use only.
 */
static void _get(Queue *queue, void *item) {
    memcpy(item, &queue->item_buffer[queue->read_idx * queue->item_size],
           queue->item_size);
    queue->read_idx = (queue->read_idx + 1) % queue->max_items;
    queue->count--;
}

static bool _is_full(Queue const *queue) { return queue->count == queue->max_items; }

static bool _is_empty(Queue const *queue) { return queue->count == 0; }

static size_t _item_count(Queue const *queue) { return queue->count; }

Queue *queue_create_static(Queue *queue, size_t const num_items, size_t const item_size,
                           uint8_t *item_buffer) {
    queue->item_buffer = item_buffer;
    queue->count = 0;
    queue->read_idx = 0;
    queue->write_idx = 0;
    queue->item_size = item_size;
    queue->max_items = num_items;
    return queue;
}

Queue *queue_create(size_t const num_items, size_t const item_size) {
    Queue *queue = malloc(sizeof(Queue));
    if (queue == NULL) {
        /* No memory. */
        return NULL;
    }
    uint8_t *item_buffer = malloc(num_items * item_size);
    if (item_buffer == NULL) {
        /* No memory. */
        free(queue);
        return NULL;
    }

    return queue_create_static(queue, num_items, item_size, item_buffer);
}

void queue_free(Queue *queue) {
    if (queue == NULL) {
        /* Cannot free null pointer, need a non-null to free the internal buffer. */
        return;
    }

    if (queue->item_buffer != NULL) {
        free(queue->item_buffer);
    }

    free(queue);
}

size_t queue_item_count(Queue const *queue) {
    platform_enter_critical_section();
    size_t const item_count = _item_count(queue);
    platform_exit_critical_section();
    return item_count;
}

bool queue_is_full(Queue const *queue) {
    platform_enter_critical_section();
    bool const is_full = _is_full(queue);
    platform_exit_critical_section();
    return is_full;
}

bool queue_is_empty(Queue const *queue) {
    platform_enter_critical_section();
    bool const is_empty = _is_empty(queue);
    platform_exit_critical_section();
    return is_empty;
}

Result queue_raw_put(Queue *queue, void const *item) {
    if (queue_is_full(queue)) {
        return RES_QUEUE_FULL;
    }

    _put(queue, item);
    return RES_OK;
}

Result queue_raw_get(Queue *queue, void *item) {
    if (queue_is_empty(queue)) {
        return RES_QUEUE_EMPTY;
    }

    _get(queue, item);

    return RES_OK;
}

Result queue_put(Queue *queue, void const *item, PlatformTicks const timeout) {

    Coro *coro = context_get_coro();
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

Result queue_get(Queue *queue, void *item, PlatformTicks const timeout) {
    Coro *coro = context_get_coro();
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

Result queue_put_no_wait(Queue *queue, void const *item) {
    Result notify_result = RES_OK;
    Scheduler *scheduler = context_get_scheduler();
    bool put_success = false;

    platform_enter_critical_section();
    if (!_is_full(queue)) {
        _put(queue, item);
        put_success = true;
    }
    platform_exit_critical_section();

    if (put_success) {
        CoroEventSource const event = {.type = CORO_EVTSRC_QUEUE_PUT,
                                       .params.subject = queue};
        notify_result = scheduler_notify(scheduler, &event);
    }

    if (notify_result != RES_OK) {
        /* Critical failure to notify scheduler. */
        return RES_NOTIFY_FAILED;
    }

    return (put_success) ? RES_OK : RES_QUEUE_FULL;
}

Result queue_get_no_wait(Queue *queue, void *item) {
    Result notify_result = RES_OK;
    Scheduler *scheduler = context_get_scheduler();
    bool get_success = false;

    platform_enter_critical_section();
    if (!_is_empty(queue)) {
        _get(queue, item);
        get_success = true;
    }
    platform_exit_critical_section();

    if (get_success) {
        CoroEventSource const event = {.type = CORO_EVTSRC_QUEUE_GET,
                                       .params.subject = queue};
        notify_result = scheduler_notify(scheduler, &event);
    }

    if (notify_result != RES_OK) {
        /* Critical failure to notify scheduler. */
        return RES_NOTIFY_FAILED;
    }

    return (get_success) ? RES_OK : RES_QUEUE_EMPTY;
}
