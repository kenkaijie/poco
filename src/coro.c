#include <poco/coro.h>
#include <string.h>

// These are reversed so they appear cute when debugging.
#define STACK_START_MAGIC (0x0DF0FECA) // 0xCAFEF00D
#define STACK_END_MAGIC (0xEFBEDABA)   // 0xBADABEEF

/*!
 * @brief Special yield mode to indicate the coroutine is done.
 *
 * This is private as no user yields should abort the coroutine. If this is needed,
 * simply return from the coroutine.
 */
static void _coro_yield_done(coro_t *coro) {
    coro->yield_signal = CORO_SIG_DONE;
    platform_swap_context(&coro->resume_context, &coro->suspend_context);
}

static void _coro_entry_point(coro_t *coro, void *context) {
    coro->entrypoint(coro, context);
    coro->coro_state = CORO_STATE_FINISHED;
    _coro_yield_done(coro);
}

static bool _update_event_sink(coro_event_sink_t *sink,
                               coro_event_source_t const *event) {
    bool unblock_task = false;

    switch (event->type) {
    case CORO_EVTSRC_ELAPSED:
        if (sink->type == CORO_EVTSINK_DELAY) {
            sink->params.ticks_remaining -= event->params.elasped_ticks;
            unblock_task = (sink->params.ticks_remaining <= 0);
        }
        break;
    case CORO_EVTSRC_QUEUE_GET:
        if (sink->type == CORO_EVTSINK_QUEUE_NOT_FULL) {
            unblock_task = (sink->params.queue == event->params.queue);
        }
        break;
    case CORO_EVTSRC_QUEUE_PUT:
        if (sink->type == CORO_EVTSINK_QUEUE_NOT_EMPTY) {
            unblock_task = (sink->params.queue == event->params.queue);
        }
        break;
    case CORO_EVTSRC_EVENT_SET:
        if (sink->type == CORO_EVTSINK_EVENT_GET) {
            unblock_task = (sink->params.event == event->params.event);
        }
        break;
    default:
        unblock_task = false;
    }
    return unblock_task;
}

coro_t *coro_create_static(coro_t *coro, coro_function_t function, void *context,
                           platform_stack_t *stack, size_t stack_size) {
    // fill the stack with a marking value
    // mark the start of the stack with a magic number,
    // and the end with another magic number, we will reduce the stack size by 2 bytes
    memset(stack, 0x55, stack_size);
    stack[0] = STACK_START_MAGIC;
    stack[stack_size / sizeof(uint32_t) - 1] = STACK_END_MAGIC;

    coro->coro_state = CORO_STATE_READY;
    coro->entrypoint = function;
    coro->stack = stack;
    coro->stack_size = stack_size;
    coro->resume_context.uc_stack.ss_sp = (void *)(stack + 1);
    coro->resume_context.uc_stack.ss_size = stack_size - (2 * sizeof(uint32_t));
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
    size_t block_count = stack_size / sizeof(platform_stack_t);
    size_t real_stack_size = sizeof(platform_stack_t) * block_count;
    platform_stack_t *stack = (platform_stack_t *)malloc(real_stack_size);

    if (stack == NULL) {
        /* No memory. */
        free(coro);
        return NULL;
    }

    coro_t *coro_handle =
        coro_create_static(coro, function, context, stack, real_stack_size);
    if (coro_handle == NULL) {
        coro_free(coro);
    }

    return coro_handle;
}

void coro_free(coro_t *coro) {
    if (coro == NULL) {
        /* cannot free null pointer */
        return;
    }

    if (coro->stack != NULL) {
        free(coro->stack);
    }

    free(coro);
}

coro_signal_t coro_resume(coro_t *coro) {
    if (coro->coro_state == CORO_STATE_FINISHED)
        return CORO_SIG_DONE;

    coro->coro_state = CORO_STATE_RUNNING;
    platform_swap_context(&coro->suspend_context, &coro->resume_context);

    switch (coro->yield_signal) {
    case CORO_SIG_NOTIFY:
        coro->coro_state = CORO_STATE_READY;
        break;
    case CORO_SIG_DONE:
        coro->coro_state = CORO_STATE_FINISHED;
        break;
    case CORO_SIG_WAIT: /* Intentional Fall-through */
    case CORO_SIG_NOTIFY_AND_WAIT:
        coro->coro_state = CORO_STATE_BLOCKED;
        break;
    }

    return coro->yield_signal;
}

void coro_yield(coro_t *coro) {
    coro->event_source.type = CORO_EVTSRC_NOOP;
    coro->yield_signal = CORO_SIG_NOTIFY;
    platform_swap_context(&coro->resume_context, &coro->suspend_context);
}

void coro_yield_delay(coro_t *coro, int64_t duration_ms) {
    coro->event_sinks[EVENT_SINK_SLOT_PRIMARY].type = CORO_EVTSINK_NONE;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].type = CORO_EVTSINK_DELAY;
    coro->event_sinks[EVENT_SINK_SLOT_TIMEOUT].params.ticks_remaining =
        duration_ms * platform_get_ticks_per_ms();

    coro->yield_signal = CORO_SIG_WAIT;
    platform_swap_context(&coro->resume_context, &coro->suspend_context);
}

void coro_yield_with_event(coro_t *coro, coro_event_source_t const *event) {
    coro->event_source = *event;
    coro->yield_signal = CORO_SIG_NOTIFY;
    platform_swap_context(&coro->resume_context, &coro->suspend_context);
}

void coro_yield_with_signal(coro_t *coro, coro_signal_t signal) {
    coro->yield_signal = signal;
    platform_swap_context(&coro->resume_context, &coro->suspend_context);
}

bool coro_notify(coro_t *coro, coro_event_source_t const *event) {
    if (coro->coro_state != CORO_STATE_BLOCKED) {
        /* Only blocked coroutines actually look for events. */
        return false;
    }

    bool unblock_task = false;

    for (size_t event_sink_idx = 0;
         (event_sink_idx < EVENT_SINK_SLOT_COUNT) && !unblock_task; ++event_sink_idx) {
        unblock_task = _update_event_sink(&coro->event_sinks[event_sink_idx], event);
    }
    /* Reset sinks, we are unblocked. */
    if (unblock_task) {
        for (size_t i = 0; i < EVENT_SINK_SLOT_COUNT; ++i) {
            coro->event_sinks[i].type = CORO_EVTSINK_NONE;
        }
        coro->coro_state = CORO_STATE_READY;
    }
    return unblock_task;
}
