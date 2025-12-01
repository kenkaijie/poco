// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Raw queue operations.
 *
 * This is only used in special circumstances, as it does not alert the scheduler in any
 * way.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <poco/queue.h>
#include <poco/result.h>

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
Result queue_raw_put(Queue *queue, void const *item);

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
Result queue_raw_get(Queue *queue, void *item);

#ifdef __cplusplus
}
#endif
