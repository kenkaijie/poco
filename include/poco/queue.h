// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Coroutine aware queue implementation.
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
enum res_code_queue {
    /*! Attempted to get item from an empty queue. */
    RES_QUEUE_EMPTY = RES_CODE(RES_GROUP_QUEUE, 0),

    /*! Attempted to put an item into a full queue. */
    RES_QUEUE_FULL = RES_CODE(RES_GROUP_QUEUE, 1),
};

typedef struct queue {
    size_t count;
    size_t read_idx;
    size_t write_idx;
    uint8_t *item_buffer;
    size_t item_size;
    size_t max_items;
} queue_t;

/*!
 * @brief Initialises a statically defined queue.
 *
 * @param queue Queue to initialise.
 * @param num_items Number of items this queue will manage.
 * @param item_size Size of an individual item, in bytes.
 * @param item_buffer Buffer containing enough space for the items.
 *
 * @return Pointer to the queue, or NULL if an error has occured.
 */
queue_t *queue_create_static(queue_t *queue, size_t num_items, size_t item_size,
                             uint8_t *item_buffer);

/*!
 * @brief Creates a queue with a fixed number of elements.
 *
 * @param num_items Number of items this queue will manage.
 * @param item_size Size of an individual item, in bytes.
 *
 * @return Pointer to the queue, or NULL if an error has occured.
 */
queue_t *queue_create(size_t num_items, size_t item_size);

/*!
 * @brief Frees a previously created queue.
 *
 * @warning Freeing a statically created queue is undefined.
 *
 * @param queue Queue to Free.
 */
void queue_free(queue_t *queue);

/*!
 * @brief Gets the number of items in a queue.
 *
 * @param queue
 *
 * @return Number of items in the queue.
 */
size_t queue_item_count(queue_t *queue);

/*!
 * @brief Check if the queue is full.
 *
 * @param queue Queue to check.
 *
 * @return True if queue is full, false otherwise.
 */
bool queue_is_full(queue_t *queue);

/*!
 * @brief Gets the number of items in a queue.
 *
 * @param queue Queue to check.
 *
 * @return True if queue is empty, false otherwise.
 */
bool queue_is_empty(queue_t *queue);

/*!
 * @brief Puts an item into the queue from a coroutine.
 *
 * Items are copied into the queue. It is up to the caller to allocate the
 * correct variable size.
 *
 * This operation blocks the calling coroutine until the item can be queued.
 *
 * @param queue Queue to put the item into.
 * @param item Item to put into the queue.
 * @param timeout Maximum time to wait before giving up.
 *
 * @retval #RES_OK on success.
 * @retval #RES_TIMEOUT if the maximum time was awaited.
 */
result_t queue_put(queue_t *queue, void const *item, platform_ticks_t timeout);

/*!
 * @brief Gets an item from the queue from a coroutine.
 *
 * Items are copied out of the queue. It is up to the caller to allocate the
 * correct variable size.
 *
 * @param queue Queue to get the item from.
 * @param item Item to get from the queue.
 * @param timeout Maximum time to wait before giving up.
 *
 * @retval #RES_OK on success.
 * @retval #RES_TIMEOUT if the maximum time was awaited.
 */
result_t queue_get(queue_t *queue, void *item, platform_ticks_t timeout);

/*!
 * @brief Puts an item without waiting.
 *
 * @param queue Queue to put the item into.
 * @param item Item to put into the queue.
 *
 * @retval #RES_OK Item has been queued.
 * @retval #RES_QUEUE_FULL Queue is full, no item has been inserted.
 * @retval #RES_NOTIFY_FAILED if the operation failed to notify the scheduler.
 */
result_t queue_put_no_wait(queue_t *queue, void const *item);

/*!
 * @brief Gets an item without waiting.
 *
 * @param queue Queue to get the item from.
 * @param item Item to get from the queue, only valid if result was #RES_OK.
 *
 * @retval #RES_OK An item has been taken.
 * @retval #RES_QUEUE_EMPTY Queue has no items to get.
 * @retval #RES_NOTIFY_FAILED if the operation failed to notify the scheduler.
 */
result_t queue_get_no_wait(queue_t *queue, void *item);

/*!
 * @brief Puts an item into the queue without notifying the scheduler.
 *
 * @warning Not notifying the scheduler may result in coroutines that are blocked
 *          forever. This is only used for queues that are not used for inter coroutine
 *          communications.
 *
 * @param queue Queue to put the item into.
 * @param item Item to put into the queue.
 *
 * @retval #RES_OK Item has been queued.
 * @retval #RES_QUEUE_FULL Queue is full, no item has been inserted.
 */
result_t queue_raw_put(queue_t *queue, void const *item);

/*!
 * @brief Gets an item from the queue without notifying the scheduler.
 *
 * @warning Not notifying the scheduler may result in coroutines that are blocked
 *          forever. This is only used for queues that are not used for inter coroutine
 *          communications.
 *
 * @param queue Queue to get the item from.
 * @param item Item to get from the queue, only valid if result was #RES_OK.
 *
 * @retval #RES_OK Item has been queued.
 * @retval #RES_QUEUE_FULL Queue is full, no item has been inserted.
 */
result_t queue_raw_get(queue_t *queue, void *item);

#ifdef __cplusplus
}
#endif
