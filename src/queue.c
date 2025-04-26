#include <poco/intracoro.h>
#include <poco/queue.h>
#include <string.h>

/*!
 * @brief Unsafe push, does not perform checking and is for internal use only.
 */
__attribute__((always_inline)) static inline void _internal_put(queue_t *queue,
                                                                void const *item) {
    memcpy(&queue->item_buffer[queue->write_idx * queue->item_size], item,
           queue->item_size);
    queue->write_idx = (queue->write_idx + 1) % queue->max_items;
    queue->count++;
}

/*!
 * @brief Unsafe pop, does not perform checking and is for internal use only.
 */
__attribute__((always_inline)) static inline void _internal_get(queue_t *queue,
                                                                void *item) {
    memcpy(item, &queue->item_buffer[queue->read_idx * queue->item_size],
           queue->item_size);
    queue->read_idx = (queue->read_idx + 1) % queue->max_items;
    queue->count--;
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
        /* Cannot free null pointer. */
        return;
    }

    if (queue->item_buffer != NULL) {
        free(queue->item_buffer);
    }

    free(queue);
}

size_t queue_item_count(queue_t *queue) { return queue->count; }

bool queue_is_full(queue_t *queue) {
    return queue_item_count(queue) == queue->max_items;
}

bool queue_is_empty(queue_t *queue) { return queue_item_count(queue) == 0; }

result_t queue_raw_put(queue_t *queue, void const *item) {
    if (queue_is_full(queue)) {
        return RES_QUEUE_FULL;
    }

    _internal_put(queue, item);
    return RES_OK;
}

result_t queue_raw_get(queue_t *queue, void *item) {
    if (queue_is_empty(queue)) {
        return RES_QUEUE_EMPTY;
    }

    _internal_get(queue, item);

    return RES_OK;
}

result_t queue_put(coro_t *coro, queue_t *queue, void const *item) {
    while (queue_is_full(queue)) {
        // Wait for space to be available

        coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type = CORO_EVTSINK_QUEUE_NOT_FULL;
        coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].params.queue = queue;
        coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_NONE;

        coro_yield_with_signal(coro, CORO_SIG_WAIT);
    }

    _internal_put(queue, item);

    coro->event_source.type = CORO_EVTSRC_QUEUE_PUT;
    coro->event_source.params.queue = queue;
    coro_yield_with_signal(coro, CORO_SIG_NOTIFY);

    return RES_OK;
}

result_t queue_get(coro_t *coro, queue_t *queue, void *item) {
    while (queue_is_empty(queue)) {
        coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type = CORO_EVTSINK_QUEUE_NOT_EMPTY;
        coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].params.queue = queue;
        coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_NONE;

        coro_yield_with_signal(coro, CORO_SIG_WAIT);
    }

    _internal_get(queue, item);

    coro->event_source.type = CORO_EVTSRC_QUEUE_GET;
    coro->event_source.params.queue = queue;
    coro_yield_with_signal(coro, CORO_SIG_NOTIFY);

    return RES_OK;
}

result_t queue_put_no_wait(coro_t *coro, queue_t *queue, void const *item) {
    if (queue_is_full(queue)) {
        return RES_QUEUE_FULL;
    }

    _internal_put(queue, item);

    coro->event_source.type = CORO_EVTSRC_QUEUE_PUT;
    coro->event_source.params.queue = queue;
    coro_yield_with_signal(coro, CORO_SIG_NOTIFY);

    return RES_OK;
}

result_t queue_get_no_wait(coro_t *coro, queue_t *queue, void *item) {
    if (queue_is_empty(queue)) {
        return RES_QUEUE_EMPTY;
    }

    _internal_get(queue, item);

    coro->event_source.type = CORO_EVTSRC_QUEUE_GET;
    coro->event_source.params.queue = queue;
    coro_yield_with_signal(coro, CORO_SIG_NOTIFY);

    return RES_OK;
}
