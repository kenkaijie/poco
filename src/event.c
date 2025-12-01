// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Implementation of a coroutine safe eventing construct.
 */

#include <poco/context.h>
#include <poco/coro.h>
#include <poco/event.h>
#include <poco/intracoro.h>

Event *event_create_static(Event *event, Flags const initial) {
    event->flags = initial;
    return event;
}

Event *event_create(Flags const initial) {
    Event *event = malloc(sizeof(Event));

    if (event == NULL) {
        return event;
    }

    return event_create_static(event, initial);
}

void event_free(Event *event) { free(event); }

Flags event_get(Event *event, Flags const mask, Flags const clear_mask,
                bool const wait_for_all, PlatformTicks const timeout) {

    Coro *coro = context_get_coro();

    Flags captured_flags = 0;
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

            coro_yield_with_signal(CORO_SIG_WAIT);

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

void event_set(Event *event, Flags const mask) {

    Coro *coro = context_get_coro();

    platform_enter_critical_section();
    event->flags |= mask;
    platform_exit_critical_section();

    coro->event_source.type = CORO_EVTSRC_EVENT_SET;
    coro->event_source.params.subject = event;
    coro_yield_with_signal(CORO_SIG_NOTIFY);
}

Result event_set_from_isr(Event *event, Flags const mask) {
    Result notify_result = RES_OK;
    Scheduler *scheduler = context_get_scheduler();

    event->flags |= mask;

    CoroEventSource const event_source = {
        .type = CORO_EVTSRC_EVENT_SET,
        .params.subject = event,
    };
    notify_result = scheduler_notify_from_isr(scheduler, &event_source);
    return (notify_result == RES_OK) ? RES_OK : RES_NOTIFY_FAILED;
}
