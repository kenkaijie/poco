/*!
 * @file
 * @brief Base coroutine implementation.
 *
 * SPDX-FileCopyrightText: Copyright contributors to the poco project.
 * SPDX-License-Identifier: MIT
 */

#include <poco/context.h>
#include <poco/coro.h>
#include <string.h>

// These are reversed so they appear cute when debugging.
#define STACK_START_MAGIC (0x0DF0FECA) // 0xCAFEF00D
#define STACK_END_MAGIC (0xEFBEADBA)   // 0xBAADBEEF
#define STACK_PAINT_MAGIC (0x55)

static void _coro_entry_point(coro_t *coro, void *context) {
    coro->entrypoint(context);

    /** Coroutine is finished, setup events. */
    coro->event_source.type = CORO_EVTSRC_CORO_FINISHED;
    coro->event_source.params.subject = coro;
    coro_yield_with_signal(CORO_SIG_NOTIFY_AND_DONE);
}

static bool _update_event_sink(coro_event_sink_t *sink,
                               coro_event_source_t const *event) {
    bool unblock_task = false;

    switch (event->type) {
    case CORO_EVTSRC_ELAPSED:
        if (sink->type == CORO_EVTSINK_DELAY &&
            sink->params.ticks_remaining != PLATFORM_TICKS_FOREVER) {
            /* We need to consider both signed and unsigned cases. */
            if (event->params.elasped_ticks > sink->params.ticks_remaining) {
                sink->params.ticks_remaining = 0;
            } else {
                sink->params.ticks_remaining -= event->params.elasped_ticks;
            }
            sink->params.ticks_remaining -= event->params.elasped_ticks;
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
    default:
        unblock_task = false;
    }
    return unblock_task;
}

coro_t *coro_create_static(coro_t *coro, coro_function_t function, void *context,
                           platform_stack_t *stack, size_t stack_size) {
    // paint the stack with 0x55s
    // mark the start of the stack with a magic number,
    // and the end with another magic number, we will reduce the stack size by 2 bytes
    memset(stack, STACK_PAINT_MAGIC, stack_size);
    stack[0] = STACK_START_MAGIC;
    stack[stack_size - 1] = STACK_END_MAGIC;

    coro->coro_state = CORO_STATE_READY;
    coro->entrypoint = function;
    coro->stack = stack;
    coro->stack_size = stack_size;
    coro->resume_context.uc_stack.ss_sp = (void *)(stack + 1);
    coro->resume_context.uc_stack.ss_size = (stack_size - 2) * sizeof(platform_stack_t);
    coro->resume_context.uc_link = 0;

    platform_get_context(&coro->resume_context);
    platform_make_context(&coro->resume_context, _coro_entry_point, coro, context);
    return coro;
}

coro_t *coro_create(coro_function_t function, void *context, size_t stack_size) {
    coro_t *coro = (coro_t *)malloc(sizeof(coro_t));
    if (coro == NULL) {
        /* No memory */
        return NULL;
    }

    // we can only create items which are multiples of platform_stack_t.
    size_t const stack_size_bytes = sizeof(platform_stack_t) * stack_size;
    platform_stack_t *stack = (platform_stack_t *)malloc(stack_size_bytes);

    if (stack == NULL) {
        /* No memory. */
        free(coro);
        return NULL;
    }

    coro_t *coro_handle =
        coro_create_static(coro, function, context, stack, stack_size_bytes);
    if (coro_handle == NULL) {
        coro_free(coro);
    }

    return coro_handle;
}

void coro_free(coro_t *coro) {
    if (coro == NULL) {
        /* cannot free null pointer, as we cannot access the stack to free it first. */
        return;
    }

    if (coro->stack != NULL) {
        free(coro->stack);
    }

    free(coro);
}

void coro_yield(void) {
    coro_t *coro = context_get_coro();
    coro->event_source.type = CORO_EVTSRC_NOOP;
    coro->yield_signal = CORO_SIG_NOTIFY;
    platform_swap_context(&coro->resume_context, &coro->suspend_context);
}

void coro_yield_delay(int64_t duration_ms) {
    coro_t *coro = context_get_coro();
    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type = CORO_EVTSINK_NONE;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_DELAY;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].params.ticks_remaining =
        duration_ms * platform_get_ticks_per_ms();

    coro->yield_signal = CORO_SIG_WAIT;
    platform_swap_context(&coro->resume_context, &coro->suspend_context);
}

void coro_yield_with_event(coro_event_source_t const *event) {
    coro_t *coro = context_get_coro();
    coro->event_source = *event;
    coro->yield_signal = CORO_SIG_NOTIFY;
    platform_swap_context(&coro->resume_context, &coro->suspend_context);
}

void coro_yield_with_signal(coro_signal_t signal) {
    coro_t *coro = context_get_coro();
    coro->yield_signal = signal;
    platform_swap_context(&coro->resume_context, &coro->suspend_context);
}

bool coro_notify(coro_t *coro, coro_event_source_t const *event) {
    if (coro->coro_state != CORO_STATE_BLOCKED) {
        /* Only blocked coroutines actually look for events. */
        return false;
    }

    bool unblock_task = false;

    for (size_t idx = 0; (idx < EVENT_SINK_SLOT_COUNT); ++idx) {
        unblock_task = _update_event_sink(&coro->event_sinks[idx], event);
        if (unblock_task) {
            coro->triggered_event_sink_slot = idx;
            coro->coro_state = CORO_STATE_READY;
            break;
        }
    }
    /* Reset sinks, we are unblocked. */
    return unblock_task;
}

coro_signal_t coro_resume(coro_t *coro) {
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

void coro_join(coro_t *coro) {
    coro_t *this_coro = context_get_coro();
    if (coro->coro_state == CORO_STATE_FINISHED) {
        /* Already done. */
        coro_yield();
        return;
    }

    this_coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type = CORO_EVTSINK_WAIT_FINISH;
    this_coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].params.subject = coro;
    this_coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_NONE;

    coro_yield_with_signal(CORO_SIG_WAIT);

    return;
}
