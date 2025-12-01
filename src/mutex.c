// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Implementation for mutexes.
 */

#include <poco/context.h>
#include <poco/coro.h>
#include <poco/mutex.h>

Mutex *mutex_create_static(Mutex *mutex) {
    mutex->owner = NULL;
    return mutex;
}

Mutex *mutex_create(void) {
    Mutex *mutex = (Mutex *)malloc(sizeof(Mutex));

    if (mutex == NULL) {
        /* No memory. */
        return NULL;
    }

    return mutex_create_static(mutex);
}

void mutex_free(Mutex *mutex) { free(mutex); }

Result mutex_acquire(Mutex *mutex, PlatformTick timeout) {
    Coro *coro = context_get_coro();
    bool acquire_success = false;

    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type = CORO_EVTSINK_MUTEX_ACQUIRE;
    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].params.subject = mutex;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_DELAY;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].params.ticks_remaining = timeout;

    while (!acquire_success) {

        if (mutex->owner == NULL) {
            mutex->owner = coro;
            acquire_success = true;
        }

        if (!acquire_success) {
            coro_yield_with_signal(CORO_SIG_WAIT);

            if (coro->triggered_event_sink_slot == EVENT_SINK_SLOT_TIMEOUT) {
                /* Timeout. */
                break;
            }
        }
    }

    return (acquire_success) ? RES_OK : RES_TIMEOUT;
}

Result mutex_acquire_no_wait(Mutex *mutex) {
    // no need locks, mutex operations should only happen within coroutines.
    bool acquired = false;
    Coro *coro = context_get_coro();

    if (mutex->owner != NULL) {
        mutex->owner = coro;
        acquired = true;
    }

    return (acquired) ? RES_OK : RES_MUTEX_OCCUPIED;
}

Result mutex_release(Mutex *mutex) {
    Coro *coro = context_get_coro();

    if ((mutex->owner != NULL) && (mutex->owner != coro)) {
        return RES_MUTEX_NOT_OWNER;
    }

    mutex->owner = NULL;

    CoroEventSource const event_source = {.type = CORO_EVTSRC_MUTEX_RELEASE,
                                          .params.subject = mutex};

    coro_yield_with_event(&event_source);

    return RES_OK;
}