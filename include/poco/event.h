// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Event communication primitive.
 *
 * Enables coroutines to signal each other in a light weight manner, as compared to
 * the queue API.
 *
 * These are designed to be a multi producer and single consumer, where the consumer is
 * a coroutine.
 *
 * An event comprises of 32 bit flags. Each can be set. It is expected the consumer is
 * responsible for clearing the bits, while the producer is responsible for setting
 * them.
 *
 * A producer can only set bits, it cannot clear them.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <poco/platform.h>
#include <poco/scheduler.h>
#include <stdint.h>

typedef uint32_t flags_t;

typedef struct event {
    flags_t flags;
} event_t;

event_t *event_create_static(event_t *event, flags_t initial);

event_t *event_create(flags_t initial);

void event_free(event_t *event);

/*!
 * @brief Waits on the specific event flags.
 *
 * @param event Event to wait on.
 * @param mask Mask to wait on, can be used to ignore flags.
 * @param clear_mask Flags to clear after the wait has finished.
 * @param wait_for_all If true, all flags must be set before the coroutine will yield.
 *                     This effective sets if the mask uses OR or AND for its wait
 *                     logic.
 * @param timeout Number of ticks to wait before timing out.
 *
 * @returns The flags that ended the wait. If flags are all 0, an error has occured.
 */
flags_t event_get(event_t *event, flags_t mask, flags_t clear_mask, bool wait_for_all,
                  platform_ticks_t timeout);

/*!
 * @brief Sets the event flags.
 *
 * @param event Event to set.
 * @param mask Mask to set.
 */
void event_set(event_t *event, flags_t mask);

/*!
 * @brief Sets the event flags from an ISR.
 *
 * The responsiveness of the waking coroutine will depend on the scheduler
 * implementation.
 *
 * @param event Event to set.
 * @param mask Mask to set.
 *
 * @param #RES_OK if the event was set.
 * @param #RES_NOTIFY_FAILED if the scheduler could not be notified. This is a critical
 *      error.
 */
result_t event_set_from_isr(event_t *event, flags_t mask);

#ifdef __cplusplus
}
#endif
