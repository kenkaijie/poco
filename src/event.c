#include <poco/event.h>
#include <poco/intracoro.h>

event_t *event_create_static(event_t *event, flags_t initial) {
    event->flags = initial;
    return event;
}

event_t *event_create(flags_t initial) {
    event_t *event = (event_t *)malloc(sizeof(event_t));

    if (event == NULL) {
        return event;
    }

    return event_create_static(event, initial);
}

void event_free(event_t *event) { free(event); }

flags_t event_get(coro_t *coro, event_t *event, flags_t mask, flags_t clear_mask,
                  bool wait_for_all, platform_ticks_t timeout) {

    flags_t captured_flags = 0;
    bool event_triggered = false;

    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type = CORO_EVTSINK_EVENT_GET;
    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].params.subject = event;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_DELAY;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].params.ticks_remaining = timeout;

    while (!event_triggered) {

        // No need critical section here, flags cannot be cleared outside of this
        // function once it has been set.
        event_triggered = (wait_for_all) ? ((event->flags & mask) == mask)
                                         : ((event->flags & mask) != 0);

        if (!event_triggered) {

            coro_yield_with_signal(coro, CORO_SIG_WAIT);

            if (coro->triggered_event_sink_slot == EVENT_SINK_SLOT_TIMEOUT) {
                break;
            }
        }
    }

    if (event_triggered) {

        platform_enter_critical_section();
        captured_flags = event->flags & mask;
        event->flags &= ~clear_mask;
        platform_exit_critical_section();
    }

    return captured_flags;
}

void event_set(coro_t *coro, event_t *event, flags_t mask) {
    platform_enter_critical_section();
    event->flags |= mask;
    platform_exit_critical_section();

    coro->event_source.type = CORO_EVTSRC_EVENT_SET;
    coro->event_source.params.subject = event;
    coro_yield_with_signal(coro, CORO_SIG_NOTIFY);
}

void event_set_from_ISR(scheduler_t *scheduler, event_t *event, flags_t mask) {
    platform_enter_critical_section();
    event->flags |= mask;
    platform_exit_critical_section();

    coro_event_source_t event_source = {
        .type = CORO_EVTSRC_EVENT_SET,
        .params.subject = event,
    };
    scheduler_notify_from_isr(scheduler, &event_source);
}
