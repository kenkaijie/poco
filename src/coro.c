// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Base coroutine implementation.
 */

#include <poco/context.h>
#include <poco/coro.h>
#include <poco/coro_raw.h>
#include <string.h>

// These are reversed so they appear cute when debugging.
#define STACK_START_MAGIC (0x0DF0FECA) // 0xCAFEF00D
#define STACK_END_MAGIC (0xEFBEADBA)   // 0xBAADBEEF
#define STACK_PAINT_MAGIC (0x55)

static void enter_coro(Coro *coro, void *context) {
    coro->entrypoint(context);

    /** Coroutine is finished, setup events. */
    coro->event_source.type = CORO_EVTSRC_CORO_FINISHED;
    coro->event_source.params.subject = coro;
    coro_yield_with_signal(CORO_SIG_NOTIFY_AND_DONE);
}

static bool update_event_sink(CoroEventSink *sink, CoroEventSource const *event) {
    bool unblock_task = false;

    switch (event->type) {
    case CORO_EVTSRC_ELAPSED:
        if (sink->type == CORO_EVTSINK_DELAY &&
            sink->params.ticks_remaining != PLATFORM_TICKS_FOREVER) {
            /* We need to consider both signed and unsigned cases. */
            if (event->params.elapsed_ticks > sink->params.ticks_remaining) {
                sink->params.ticks_remaining = 0;
            } else {
                sink->params.ticks_remaining -= event->params.elapsed_ticks;
            }
            sink->params.ticks_remaining -= event->params.elapsed_ticks;
            unblock_task = (sink->params.ticks_remaining <= 0);
        }
        break;
    case CORO_EVTSRC_QUEUE_GET:
        if (sink->type == CORO_EVTSINK_QUEUE_NOT_FULL) {
            unblock_task = (sink->params.subject == event->params.subject);
        }
        break;
    case CORO_EVTSRC_QUEUE_PUT:
        if (sink->type == CORO_EVTSINK_QUEUE_NOT_EMPTY) {
            unblock_task = (sink->params.subject == event->params.subject);
        }
        break;
    case CORO_EVTSRC_EVENT_SET:
        if (sink->type == CORO_EVTSINK_EVENT_GET) {
            unblock_task = (sink->params.subject == event->params.subject);
        }
        break;
    case CORO_EVTSRC_SEMAPHORE_RELEASE:
        if (sink->type == CORO_EVTSINK_SEMAPHORE_ACQUIRE) {
            unblock_task = (sink->params.subject == event->params.subject);
        }
        break;
    case CORO_EVTSRC_MUTEX_RELEASE:
        if (sink->type == CORO_EVTSINK_MUTEX_ACQUIRE) {
            unblock_task = (sink->params.subject == event->params.subject);
        }
        break;
    case CORO_EVTSRC_CORO_FINISHED:
        if (sink->type == CORO_EVTSINK_WAIT_FINISH) {
            unblock_task = (sink->params.subject == event->params.subject);
        }
        break;
    case CORO_EVTSRC_STREAM_RECV:
        if (sink->type == CORO_EVTSINK_STREAM_NOT_FULL) {
            unblock_task = (sink->params.subject == event->params.subject);
        }
        break;
    case CORO_EVTSRC_STREAM_SEND:
        if (sink->type == CORO_EVTSINK_STREAM_NOT_EMPTY) {
            unblock_task = (sink->params.subject == event->params.subject);
        }
        break;
    default:
        unblock_task = false;
    }
    return unblock_task;
}

Coro *coro_create_static(Coro *coro, CoroEntrypoint const entrypoint, void *context,
                         PlatformStackElement *stack, size_t const stack_count) {

    if (stack_count < 3) {
        /* need at least 3 elements: start magic, usable stack, end magic */
        return NULL;
    }

    // paint the stack with 0x55s
    // mark the start of the stack with a magic number,
    // and the end with another magic number, we will reduce the stack size by 2 bytes
    memset(stack, STACK_PAINT_MAGIC, stack_count);
    stack[0] = STACK_START_MAGIC;
    stack[stack_count - 1] = STACK_END_MAGIC;

    coro->coro_state = CORO_STATE_READY;
    coro->entrypoint = entrypoint;
    coro->stack = stack;
    coro->stack_size = stack_count;
    coro->resume_context.uc_stack.ss_sp = (void *)(stack + 1);
    coro->resume_context.uc_stack.ss_size =
        (stack_count - 2) * sizeof(PlatformStackElement);
    coro->resume_context.uc_link = 0;

    memset(&coro->suspend_context, 0, sizeof(coro->suspend_context));

    platform_get_context(&coro->resume_context);
    platform_make_context(&coro->resume_context, (void (*)(void *, void *))enter_coro,
                          coro, context);
    return coro;
}

Coro *coro_create(CoroEntrypoint const entrypoint, void *context,
                  size_t const stack_count) {
    Coro *coro = malloc(sizeof(Coro));
    if (coro == NULL) {
        /* No memory */
        return NULL;
    }

    // we can only create items which are multiples of PlatformStackElement.
    PlatformStackElement *stack = malloc(sizeof(PlatformStackElement) * stack_count);

    if (stack == NULL) {
        /* No memory. */
        free(coro);
        return NULL;
    }

    Coro *coro_handle = coro_create_static(coro, entrypoint, context, stack, stack_count);
    if (coro_handle == NULL) {
        free(coro);
        free(stack);
    }

    return coro_handle;
}

void coro_destroy_static(Coro *coro) {
    platform_destroy_context(&coro->resume_context);
    platform_destroy_context(&coro->suspend_context);
}

void coro_free(Coro *coro) {
    if (coro == NULL) {
        /* cannot free null pointer, as we cannot access the stack to free it first. */
        return;
    }

    if (coro->stack != NULL) {
        free(coro->stack);
    }

    coro_destroy_static(coro);

    free(coro);
}

void coro_yield(void) {
    Coro *coro = context_get_coro();
    coro->event_source.type = CORO_EVTSRC_NOOP;
    coro->yield_signal = CORO_SIG_NOTIFY;
    platform_swap_context(&coro->resume_context, &coro->suspend_context);
}

void coro_yield_delay(int64_t const duration_ms) {
    Coro *coro = context_get_coro();
    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type = CORO_EVTSINK_NONE;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_DELAY;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].params.ticks_remaining =
        duration_ms * platform_get_ticks_per_ms();

    coro->yield_signal = CORO_SIG_WAIT;
    platform_swap_context(&coro->resume_context, &coro->suspend_context);
}

void coro_yield_with_event(CoroEventSource const *event) {
    Coro *coro = context_get_coro();
    coro->event_source = *event;
    coro->yield_signal = CORO_SIG_NOTIFY;
    platform_swap_context(&coro->resume_context, &coro->suspend_context);
}

void coro_yield_with_signal(CoroSignal const signal) {
    Coro *coro = context_get_coro();
    coro->yield_signal = signal;
    platform_swap_context(&coro->resume_context, &coro->suspend_context);
}

bool coro_notify(Coro *coro, CoroEventSource const *event) {
    if (coro->coro_state != CORO_STATE_BLOCKED) {
        /* Only blocked coroutines actually look for events. */
        return false;
    }

    bool unblock_task = false;

    for (size_t idx = 0; (idx < EVENT_SINK_SLOT_COUNT); ++idx) {
        unblock_task = update_event_sink(&coro->event_sinks[idx], event);
        if (unblock_task) {
            coro->triggered_event_sink_slot = idx;
            coro->coro_state = CORO_STATE_READY;
            break;
        }
    }
    /* Reset sinks, we are unblocked. */
    return unblock_task;
}

CoroSignal coro_resume(Coro *coro) {
    if (coro->coro_state == CORO_STATE_FINISHED)
        return CORO_SIG_NOTIFY_AND_DONE;

    coro->coro_state = CORO_STATE_RUNNING;
    platform_swap_context(&coro->suspend_context, &coro->resume_context);

    switch (coro->yield_signal) {
    case CORO_SIG_NOTIFY:
        coro->coro_state = CORO_STATE_READY;
        break;
    case CORO_SIG_NOTIFY_AND_DONE:
        coro->coro_state = CORO_STATE_FINISHED;
        break;
    case CORO_SIG_WAIT: /* Intentional Fall-through */
    case CORO_SIG_NOTIFY_AND_WAIT:
        coro->coro_state = CORO_STATE_BLOCKED;
        break;
    }

    return coro->yield_signal;
}

void coro_join(Coro *coro) {
    Coro *this_coro = context_get_coro();
    if (coro->coro_state == CORO_STATE_FINISHED) {
        /* Already done. */
        coro_yield();
        return;
    }

    this_coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type = CORO_EVTSINK_WAIT_FINISH;
    this_coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].params.subject = coro;
    this_coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_NONE;

    coro_yield_with_signal(CORO_SIG_WAIT);
}
