/*!
 * @file
 * @brief Event communication primitive.
 *
 * Enables coroutines to signal each other in a light weight manner, as compared to
 * the queue API.
 *
 * These are designed to be a multi producer and single consumer.
 *
 * An event comprises of 32 bit flags. Each can be set. It is expected the consumer is
 * responsible for clearing the bits, while the producer is responsible for setting
 * them.
 *
 * If the producer sets and clears a bit before the consumer can read it, the
 * consumer will not detect the event.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <poco/coro.h>
#include <poco/error.h>
#include <poco/scheduler.h>
#include <stdint.h>

typedef uint32_t flags_t;

typedef struct event {
    flags_t flags;
} event_t;

event_t *event_create_static(event_t *event, flags_t initial);

/*!
 * @brief Waits on the specific event flags.a64l
 *
 * @param coro Coroutine that is waiting on the event.
 * @param event Event to wait on.
 * @param mask Mask to wait on, can be used to ignore flags.
 * @param clear_mask Flags to clear after the wait has finished.
 * @param wait_for_all If true, all flags must be set before the coroutine will yield.
 *                     This effective sets if the mask uses OR or AND for its wait
 *                     logic.
 *
 * @returns The flags that ended the wait.
 */
flags_t event_get(coro_t *coro, event_t *event, flags_t mask, flags_t clear_mask,
                  bool wait_for_all);

/*!
 * @brief Sets the event flags.
 *
 * @param coro The currently running coroutine.
 * @param event Event to set.
 * @param mask Mask to set.
 */
void event_set(coro_t *coro, event_t *event, flags_t mask);

/*!
 * @brief Sets the event flags from an ISR.
 *
 * The responsiveness of the waking coroutine will depend on the scheduler
 * implementation.
 *
 * @param scheduler Scheduler running the corotuines.
 * @param event Event to set.
 * @param mask Mask to set.
 */
void event_set_from_ISR(scheduler_t *scheduler, event_t *event, flags_t mask);

#ifdef __cplusplus
}
#endif
