#include "observable_u32.h"
#include <poco/coro_raw.h>

/*! Special magic value for the observable custom primitive. */
#define OBSERVABLE_U32_EVENT_MAGIC (0x8AFF7321U)

static inline bool _is_target_reached(ObservableU32 const *observable) {
    return observable->target == observable->value;
}

static bool _can_unblock(CustomEventSink const *sink, CustomEventSource const *source) {
    ObservableU32 const *observable = (ObservableU32 const *)(sink->subject);
    return _is_target_reached(observable);
}

ObservableU32 *observable_create_static(ObservableU32 *observable,
                                        uint32_t initial_value) {
    observable->value = initial_value;
    observable->target = 0;
    return observable;
}

void observable_set_value(ObservableU32 *observable, uint32_t value) {

    observable->value = value;

    CoroEventSource const event_source = {
        .type = CORO_EVTSRC_CUSTOM,
        .params.custom_source = {.event_magic = OBSERVABLE_U32_EVENT_MAGIC,
                                 .subject = observable}};

    coro_yield_with_event(&event_source);
}

Result observable_wait_until(ObservableU32 *observable, uint32_t target,
                             PlatformTick timeout) {
    Coro *coro = context_get_coro();
    bool event_triggered = false;

    observable->target = target;

    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type = CORO_EVTSINK_CUSTOM;
    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].params.custom_sink.event_magic =
        OBSERVABLE_U32_EVENT_MAGIC;
    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].params.custom_sink.subject = observable;
    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].params.custom_sink.can_unblock =
        _can_unblock;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_DELAY;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].params.ticks_remaining = timeout;

    while (!event_triggered) {

        // No need critical section here, flags cannot be cleared outside of this
        // function once it has been set, and we specify single consumer.
        event_triggered = _is_target_reached(observable);

        if (!event_triggered) {

            coro_yield_with_signal(CORO_SIG_WAIT);

            if (coro->triggered_event_sink_slot == EVENT_SINK_SLOT_TIMEOUT) {
                break;
            }
        }
    }

    return (coro->triggered_event_sink_slot == EVENT_SINK_SLOT_TIMEOUT) ? RES_TIMEOUT
                                                                        : RES_OK;
}
