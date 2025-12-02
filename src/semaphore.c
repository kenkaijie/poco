// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Implementation for semaphore.
 */

#include <poco/context.h>
#include <poco/coro_raw.h>
#include <poco/semaphore.h>

Semaphore *semaphore_create_binary() { return semaphore_create(1); }

Semaphore *semaphore_create_binary_static(Semaphore *semaphore) {
    return semaphore_create_static(semaphore, 1);
}

Semaphore *semaphore_create(size_t const slot_count) {
    Semaphore *semaphore = malloc(sizeof(Semaphore));
    if (semaphore == NULL) {
        /* Malloc failure. */
        return semaphore;
    }

    return semaphore_create_static(semaphore, slot_count);
}

Semaphore *semaphore_create_static(Semaphore *semaphore, size_t slot_count) {
    semaphore->slots_remaining = slot_count;
    semaphore->slot_count = slot_count;

    return semaphore;
}

void semaphore_free(Semaphore *semaphore) { free(semaphore); }

Result semaphore_acquire(Semaphore *semaphore, PlatformTick const delay_ticks) {
    bool acquired = false;
    Coro *coro = context_get_coro();

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

Result semaphore_acquire_no_wait(Semaphore *semaphore) {
    bool acquired = false;

    platform_enter_critical_section();
    if (semaphore->slots_remaining != 0) {
        semaphore->slots_remaining--;
        acquired = true;
    }
    platform_exit_critical_section();

    return (acquired) ? RES_OK : RES_SEMAPHORE_FULL;
}

Result semaphore_acquire_from_isr(Semaphore *semaphore) {
    bool acquired = false;

    if (semaphore->slots_remaining != 0) {
        semaphore->slots_remaining--;
        acquired = true;
    }

    return (acquired) ? RES_OK : RES_TIMEOUT;
}

Result semaphore_release(Semaphore *semaphore) {
    bool released = false;

    platform_enter_critical_section();
    if (semaphore->slots_remaining != semaphore->slot_count) {
        semaphore->slots_remaining++;
        released = true;
    }
    platform_exit_critical_section();

    if (released) {
        CoroEventSource const event = {.type = CORO_EVTSRC_SEMAPHORE_RELEASE,
                                       .params.subject = semaphore};
        coro_yield_with_event(&event);
    }

    return (released) ? RES_OK : RES_OVERFLOW;
}

Result semaphore_release_from_isr(Semaphore *semaphore) {
    bool released = false;
    Result notify_result = RES_OK;
    Scheduler *scheduler = context_get_scheduler();

    if (semaphore->slots_remaining != semaphore->slot_count) {
        semaphore->slots_remaining++;
        released = true;
    }

    if (released) {
        CoroEventSource const event = {.type = CORO_EVTSRC_SEMAPHORE_RELEASE,
                                       .params.subject = semaphore};
        notify_result = scheduler_notify_from_isr(scheduler, &event);
    }

    if (notify_result != RES_OK) {
        /* Critical failure to notify scheduler. */
        return RES_NOTIFY_FAILED;
    }

    return (released) ? RES_OK : RES_OVERFLOW;
}
