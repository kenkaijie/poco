#include <poco/event.h>
#include <poco/intracoro.h>
#include <poco/platform.h>

static bool _event_triggered(event_t *event, flags_t mask, bool wait_for_all) {
    platform_enter_critical_section();
    return (wait_for_all) ? ((event->flags & mask) == mask)
                          : ((event->flags & mask) != 0);
    platform_exit_critical_section();
}

event_t *event_create_static(event_t *event, flags_t initial) {
    event->flags = initial;
    return event;
}

flags_t event_get(coro_t *coro, event_t *event, flags_t mask, flags_t clear_mask,
                  bool wait_for_all) {
    while (!_event_triggered(event, mask, wait_for_all)) {
        coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type = CORO_EVTSINK_EVENT_GET;
        coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].params.event = event;
        coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_NONE;

        coro_yield_with_signal(coro, CORO_SIG_WAIT);
    }

    flags_t captured_flags = 0;

    platform_enter_critical_section();
    captured_flags = event->flags & mask;
    event->flags &= ~clear_mask;
    platform_exit_critical_section();

    return captured_flags;
}

void event_set(coro_t *coro, event_t *event, flags_t mask) {
    platform_enter_critical_section();
    event->flags |= mask;
    platform_exit_critical_section();

    coro->event_source.type = CORO_EVTSRC_EVENT_SET;
    coro->event_source.params.event = event;
    coro_yield_with_signal(coro, CORO_SIG_NOTIFY);
}

void event_set_from_ISR(scheduler_t *scheduler, event_t *event, flags_t mask) {
    platform_enter_critical_section();
    event->flags |= mask;
    platform_exit_critical_section();

    coro_event_source_t event_source = {
        .type = CORO_EVTSRC_EVENT_SET,
        .params.event = event,
    };
    scheduler_notify_from_isr(scheduler, &event_source);
}
