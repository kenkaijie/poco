/*!
 * @file
 * @brief Semaphore communication primitive.
 *
 * Supports both binary and counting semaphores.
 */

#pragma once

#include <poco/coro.h>
#include <poco/event.h>
#include <poco/platform.h>
#include <poco/result.h>
#include <stddef.h>

typedef struct semaphore {
    size_t slots_remaining;
    size_t slot_count;
} semaphore_t;

/*!
 * @brief Create a binary sempahore of a particular size.
 *
 * @note This is equivalent to creating a sempahore with max_count set to 1.
 *
 * @return the created semaphore, or NULL.
 */
semaphore_t *semaphore_create_binary() { return semaphore_create(1); }

/*!
 * @brief Create a bounded sempahore of a particular size.
 *
 * @param max_count Maximum number of allowed concurrent acquisitions.
 *
 * @return the created semaphore, or NULL.
 */
semaphore_t *semaphore_create(size_t slot_count) {
    semaphore_t *semaphore = (semaphore_t *)malloc(sizeof(semaphore_t));
    if (semaphore == NULL) {
        /* Malloc failure. */
        return semaphore;
    }

    semaphore->slots_remaining = slot_count;
    semaphore->slot_count = slot_count;
}

/*!
 * @brief Acquire the semaphore, waiting forever.
 *
 * @param coro Calling coroutine.
 * @param semaphore Semaphore to acquire.
 *
 * @retval #RES_OK This blocks forever, so will always succeed.
 */
result_t semaphore_acquire(coro_t *coro, semaphore_t *semaphore) {
    bool acquired = false;

    // spin forever.
    while (!acquired) {

        // attempt acquire.
        platform_enter_critical_section();
        if (semaphore->slots_remaining != 0) {
            semaphore->slots_remaining--;
            acquired = true;
        }
        platform_exit_critical_section();

        if (!acquired) {
            coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type =
                CORO_EVTSINK_SEMAPHORE_ACQUIRE;
            coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].params.subject = semaphore;
            coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_NONE;

            coro_yield_with_signal(coro, CORO_SIG_WAIT);
        }
    }

    return RES_OK;
}

result_t semaphore_acquire(coro_t *coro, semaphore_t *semaphore,
                           platform_ticks_t delay_ticks) {
    bool acquired = false;

    while (!acquired) {

        platform_enter_critical_section();
        if (semaphore->slots_remaining != 0) {
            semaphore->slots_remaining--;
            acquired = true;
        }
        platform_exit_critical_section();

        if (!acquired) {
            coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type =
                CORO_EVTSINK_SEMAPHORE_ACQUIRE;
            coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].params.subject = semaphore;
            coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_DELAY;
            coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].params.ticks_remaining =
                delay_ticks;

            coro_yield_with_signal(coro, CORO_SIG_WAIT);

            if (coro->triggered_event_sink_slot == EVENT_SINK_SLOT_TIMEOUT) {
                /* Timeout. */
                break;
            }
        }
    }

    return (acquired) ? RES_OK : RES_TIMEOUT;
}

/*!
 * @brief Release the semaphore.
 *
 * @param coro Calling coroutine.
 * @param semaphore Semaphore to release.
 *
 * @retval #RES_OK Semaphore has been released.
 * @retval #RES_OVERFLOW Semaphore has already hit the maximum number of releases.
 *                       (A double release has occurred.)
 */
result_t semaphore_release(coro_t *coro, semaphore_t *semaphore) {
    bool released = false;

    platform_enter_critical_section();
    if (semaphore->slots_remaining != semaphore->slot_count) {
        semaphore->slots_remaining++;
        released = true;
    }
    platform_exit_critical_section();

    if (released) {
        coro->event_source.type = CORO_EVTSRC_SEMAPHORE_RELEASE;
        coro->event_source.params.subject = semaphore;
    }

    return (released) ? RES_OK : RES_OVERFLOW;
}
