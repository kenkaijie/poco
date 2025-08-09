// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Implementation for semaphore.
 */

#include <poco/context.h>
#include <poco/semaphore.h>

semaphore_t *semaphore_create_binary() { return semaphore_create(1); }

semaphore_t *semaphore_create_binary_static(semaphore_t *semaphore) {
    return semaphore_create_static(semaphore, 1);
}

semaphore_t *semaphore_create(size_t slot_count) {
    semaphore_t *semaphore = (semaphore_t *)malloc(sizeof(semaphore_t));
    if (semaphore == NULL) {
        /* Malloc failure. */
        return semaphore;
    }

    return semaphore_create_static(semaphore, slot_count);
}

semaphore_t *semaphore_create_static(semaphore_t *semaphore, size_t slot_count) {
    semaphore->slots_remaining = slot_count;
    semaphore->slot_count = slot_count;

    return semaphore;
}

void semaphore_free(semaphore_t *semaphore) { free(semaphore); }

result_t semaphore_acquire(semaphore_t *semaphore, platform_ticks_t delay_ticks) {
    bool acquired = false;
    coro_t *coro = context_get_coro();

    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type = CORO_EVTSINK_SEMAPHORE_ACQUIRE;
    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].params.subject = semaphore;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_DELAY;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].params.ticks_remaining = delay_ticks;

    while (!acquired) {

        platform_enter_critical_section();
        if (semaphore->slots_remaining != 0) {
            semaphore->slots_remaining--;
            acquired = true;
        }
        platform_exit_critical_section();

        if (!acquired) {
            coro_yield_with_signal(CORO_SIG_WAIT);

            if (coro->triggered_event_sink_slot == EVENT_SINK_SLOT_TIMEOUT) {
                /* Timeout. */
                break;
            }
        }
    }

    return (acquired) ? RES_OK : RES_TIMEOUT;
}

result_t semaphore_release(semaphore_t *semaphore) {
    bool released = false;

    platform_enter_critical_section();
    if (semaphore->slots_remaining != semaphore->slot_count) {
        semaphore->slots_remaining++;
        released = true;
    }
    platform_exit_critical_section();

    if (released) {
        coro_event_source_t const event = {.type = CORO_EVTSRC_SEMAPHORE_RELEASE,
                                           .params.subject = semaphore};
        coro_yield_with_event(&event);
    }

    return (released) ? RES_OK : RES_OVERFLOW;
}

result_t semaphore_acquire_from_isr(semaphore_t *semaphore) {
    bool acquired = false;

    if (semaphore->slots_remaining != 0) {
        semaphore->slots_remaining--;
        acquired = true;
    }

    return (acquired) ? RES_OK : RES_TIMEOUT;
}

result_t semaphore_release_from_isr(semaphore_t *semaphore) {
    bool released = false;
    result_t notify_result = RES_OK;
    scheduler_t *scheduler = context_get_scheduler();

    if (semaphore->slots_remaining != semaphore->slot_count) {
        semaphore->slots_remaining++;
        released = true;
    }

    if (released) {
        coro_event_source_t const event = {.type = CORO_EVTSRC_SEMAPHORE_RELEASE,
                                           .params.subject = semaphore};
        notify_result = scheduler_notify_from_isr(scheduler, &event);
    }

    if (notify_result != RES_OK) {
        /* Critical failure to notify scheduler. */
        return RES_NOTIFY_FAILED;
    }

    return (released) ? RES_OK : RES_OVERFLOW;
}
