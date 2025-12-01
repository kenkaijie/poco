// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Event communication primitive.
 *
 * Enables coroutines to signal each other in a lightweight manner, as compared to
 * the queue API.
 *
 * These are designed to be a multi producer and single consumer, where the consumer is
 * a coroutine.
 *
 * An event consists of 32 bit flags. Each can be set. It is expected the consumer is
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

typedef uint32_t Flags;

typedef struct event {
    Flags flags;
} Event;

Event *event_create_static(Event *event, Flags initial);

Event *event_create(Flags initial);

void event_free(Event *event);

/*!
 * @brief Waits on the specific event flags.
 *
 * @param event Event to wait on.
 * @param mask Mask to wait on, can be used to ignore flags.
 * @param clear_mask Flags to clear after the wait has finished.
 * @param wait_for_all If true, all flags must be set before the coroutine will yield.
 *                     This effective sets if the mask uses "OR" or "AND" for its wait
 *                     logic.
 * @param timeout Number of ticks to wait before timing out.
 *
 * @returns The flags that ended the wait. If flags are all 0, an error has occurred.
 */
Flags event_get(Event *event, Flags mask, Flags clear_mask, bool wait_for_all,
                PlatformTick timeout);

/*!
 * @brief Sets the event flags.
 *
 * @param event Event to set.
 * @param mask Mask to set.
 */
void event_set(Event *event, Flags mask);

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
Result event_set_from_isr(Event *event, Flags mask);

#ifdef __cplusplus
}
#endif
