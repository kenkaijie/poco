#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <poco/coro.h>
#include <poco/error.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** Attempted to get item from an empty queue. */
#define QUEUE_EMPTY MODULE_ERROR(MODULE_QUEUE, 0)

/** Attempted to put an item into a full queue. */
#define QUEUE_FULL MODULE_ERROR(MODULE_QUEUE, 1)

/*!
 * @brief Macro to define the items needed for a statically defined queue.
 *
 * @param name Name of the queue.
 * @param size Size of the queue, in items.
 * @param type Type of the items in the queue.
 */
#define QUEUE_STATIC_DEFINE(name, size, type)                                          \
    static uint8_t name##_queue_buffer[size * sizeof(type)]                            \
        __attribute__((__aligned__(sizeof(type))));                                    \
    static queue_t name##_queue;

typedef struct queue {
    size_t count;
    size_t read_idx;
    size_t write_idx;
    uint8_t *buffer;
    size_t element_size;
    size_t max_items;
} queue_t;

queue_t *queue_create_static(queue_t *queue, size_t num_elements, size_t element_size,
                             uint8_t *buffer);

size_t queue_count(queue_t *queue);

bool queue_full(queue_t *queue);

bool queue_empty(queue_t *queue);

/*!
 * @brief Puts an item into the queue from a coroutine.
 *
 * Items are copied into the queue. It is up to the caller to allocate the
 * correct variable size.
 *
 * @param coro Coroutine that is putting the item into the queue.
 * @param queue Queue to put the item into.
 * @param item Item to put into the queue.
 *
 * @retval RET_OK on success.
 */
error_t queue_put(coro_t *coro, queue_t *queue, void const *item);

/*!
 * @brief Gets an item from the queue from a coroutine.
 *
 * Items are copied out of the queue. It is up to the caller to allocate the
 * correct variable size.
 *
 * @param coro Coroutine that is getting the item from the queue.
 * @param queue Queue to get the item from.
 * @param item Item to get from the queue.
 *
 * @retval RET_OK on success.
 */
error_t queue_get(coro_t *coro, queue_t *queue, void *item);

error_t queue_put_no_wait(coro_t *coro, queue_t *queue, void const *item);
error_t queue_get_no_wait(coro_t *coro, queue_t *queue, void *item);

/*!
 * @name Advanced Queue Operations
 *
 * These functions are for advanced or niche uses, they expose the queue without any
 * coroutine notifications. Improper use may not properly unblock a waiting coroutine.
 */
/**@{*/
error_t queue_raw_put(queue_t *queue, void const *item);
error_t queue_raw_get(queue_t *queue, void *item);
/**@}*/

#ifdef __cplusplus
}
#endif