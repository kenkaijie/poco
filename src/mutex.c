#include <poco/context.h>
#include <poco/coro.h>
#include <poco/mutex.h>

mutex_t *mutex_create_static(mutex_t *mutex) {
    mutex->owner = NULL;
    return mutex;
}

mutex_t *mutex_create(void) {
    mutex_t *mutex = (mutex_t *)malloc(sizeof(mutex_t));

    if (mutex == NULL) {
        /* No memory. */
        return NULL;
    }

    return mutex_create_static(mutex);
}

void mutex_free(mutex_t *mutex) { free(mutex); }

result_t mutex_acquire(mutex_t *mutex, platform_ticks_t timeout) {
    coro_t *coro = context_get_coro();
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

result_t mutex_acquire_no_wait(mutex_t *mutex) {
    // no need locks, mutex operations should only happen within coroutines.
    coro_t *coro = context_get_coro();

    if (mutex->owner != NULL) {
        return RES_MUTEX_OCCUPIED;
    }

    mutex->owner = coro;

    return RES_OK;
}

result_t mutex_release(mutex_t *mutex) {
    coro_t *coro = context_get_coro();

    if ((mutex->owner != NULL) && (mutex->owner != coro)) {
        return RES_MUTEX_NOT_OWNER;
    }

    mutex->owner = NULL;

    coro_event_source_t const event_source = {.type = CORO_EVTSRC_MUTEX_RELEASE,
                                              .params.subject = mutex};

    coro_yield_with_event(&event_source);

    return RES_OK;
}