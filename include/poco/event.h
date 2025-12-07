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
#include <poco/result.h>
#include <stdint.h>

/*!
 * @brief Mask on all flag bits.
 */
#define EVENT_FLAGS_MASK_ALL (UINT32_MAX)

/*!
 * @brief Mask on no flag bits.
 */
#define EVENT_FLAGS_MASK_NONE (0)

typedef uint32_t Flags;

typedef struct event {
    Flags flags;
} Event;

/*!
 * @brief Initialise an event structure with the initial set of flags.
 *
 * @param event Event to initialise.
 * @param initial Initial flags to use.
 * @return Pointer to the same event passed in, or NULL if initialisation failed.
 */
Event *event_create_static(Event *event, Flags initial);

/*!
 * @brief Dynamically create an event.
 *
 * @param initial Initial flags to assign.
 * @return Pointer to the created event, or NULL if it failed to create.
 */
Event *event_create(Flags initial);

/*!
 * @brief Frees an event dynamically allocated with event_create.
 *
 * @param event Event previously created with event_create.
 */
void event_free(Event *event);

/*!
 * @brief Sets the event flags.
 *
 * @param event Event to set.
 * @param mask Mask to set.
 */
void event_set(Event *event, Flags mask);

/*!
 * @brief Sets the event flags without yielding.
 *
 * @param event Event to set.
 * @param mask Mask to set.
 *
 * @retval #RES_OK if the event was set.
 * @retval #RES_NOTIFY_FAILED if the scheduler could not be notified. This is a critical
 *      error.
 */
Result event_set_no_wait(Event *event, Flags mask);

/*!
 * @brief Sets the event flags from an ISR.
 *
 * @param event Event to set.
 * @param mask Mask to set.
 *
 * @retval #RES_OK if the event was set.
 * @retval #RES_NOTIFY_FAILED if the scheduler could not be notified. This is a critical
 *      error.
 */
Result event_set_from_isr(Event *event, Flags mask);

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
 * @returns Triggered flags. When a timeout is provided, flags may return zero if the
 *          finite timeout has elapsed. Otherwise, this function will always return a
 *          non-zero flag value.
 */
Flags event_get(Event *event, Flags mask, Flags clear_mask, bool wait_for_all,
                PlatformTick timeout);

/*!
 * @brief Inspects the event flags without waiting.
 *
 * Unlike the yielding version, we cannot specify the "OR" or "AND" behaviour, as we do
 * not block either way.
 *
 * @param event Event to inspect.
 * @param mask Mask to look for any flag bits that are set.
 * @param clear_mask Mask to apply to clear any bits after inspection. Clear mask is
 *                   only applied if the return value is non-zero.
 * @return The bits extracted from the mask. If this is 0, it means no flags were set.
 */
Flags event_get_no_wait(Event *event, Flags mask, Flags clear_mask);

/*!
 * @brief Inspects the event flags from an ISR.
 *
 * @param event Event to inspect.
 * @param mask Mask to look for any flag bits that are set.
 * @param clear_mask Mask to apply to clear any bits after inspection. Clear mask is
 *                   only applied if the return value is non-zero.
 * @return The bits extracted from the mask. If this is 0, it means no flags were set.
 */
Flags event_get_from_isr(Event *event, Flags mask, Flags clear_mask);

#ifdef __cplusplus
}
#endif
