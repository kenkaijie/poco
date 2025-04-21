#include <poco/coro.h>
#include <poco/platform.h>
#include <string.h>

// These are reversed so they appear cute when debugging.
#define STACK_START_MAGIC (0x0DF0FECA) // 0xCAFEF00D
#define STACK_END_MAGIC (0xEFBEDABA) // 0xBADABEEF

/*!
 * @brief Special yield mode to indicate the coroutine is done.
 *
 * This is private as no user yields should abort the coroutine. If this is needed,
 * simply return from the coroutine.
 */
static void coro_yield_done(coro_t *coro)
{
    coro->yield_signal.type = CORO_SIG_DONE;
    swapcontext(&coro->resume_context, &coro->suspend_context);
}

static void _coro_entry_point(coro_t *coro, void* context)
{
    coro->entrypoint(coro, context);
    coro_yield_done(coro);
}

coro_t *coro_create_static(coro_t *coro, coro_function_t function, void * context, uint32_t *stack, size_t stack_size)
{
    // fill the stack with a marking value
    // mark the start of the stack with a magic number,
    // and the end with another magic number, we will reduce the stack size by 2 bytes
    memset(stack, 0x55, stack_size);
    stack[0] = STACK_START_MAGIC;
    stack[stack_size / sizeof(uint32_t) - 1] = STACK_END_MAGIC;

    coro->coro_state = CORO_STATE_READY;
    coro->entrypoint = function;
    coro->resume_context.uc_stack.ss_sp = (void *) (stack + 1);
    coro->resume_context.uc_stack.ss_size = stack_size - (2 * sizeof(uint32_t));
    coro->resume_context.uc_link = 0;

    getcontext(&coro->resume_context);
    makecontext(&coro->resume_context, (void (*)())_coro_entry_point, 2, coro, context);
    return coro;
}

coro_signal_type_t coro_resume(coro_t *coro)
{
    if (coro->coro_state == CORO_STATE_FINISHED) return CORO_SIG_DONE;

    swapcontext(&coro->suspend_context, &coro->resume_context);
    return coro->yield_signal.type;
}

void coro_yield(coro_t *coro)
{
    coro->event_source.type = CORO_EVTSRC_NOOP;
    coro->yield_signal.type = CORO_SIG_NOTIFY;
    swapcontext(&coro->resume_context, &coro->suspend_context);
}

void coro_yield_delay(coro_t *coro, int64_t duration_ms)
{
    coro->event_sinks[0].type = CORO_EVTSINK_DELAY;
    coro->event_sinks[0].params.ticks_remaining = duration_ms * (CLOCKS_PER_SEC / 1000);
    coro->event_sinks[1].type = CORO_EVTSINK_NONE;
    coro->yield_signal.type = CORO_SIG_WAIT;
    swapcontext(&coro->resume_context, &coro->suspend_context);
}

void coro_yield_with_event(coro_t *coro, coro_event_source_t const *event)
{
    coro->event_source = *event;
    coro->yield_signal.type = CORO_SIG_NOTIFY;
    swapcontext(&coro->resume_context, &coro->suspend_context);
}

void coro_yield_with_signal(coro_t *coro, coro_signal_type_t signal)
{
    coro->yield_signal.type = signal;
    swapcontext(&coro->resume_context, &coro->suspend_context);
}

void coro_reset_sinks(coro_t *coro)
{
    for (size_t i=0; i < 2; ++i)
    {
        coro->event_sinks[i].type = CORO_EVTSINK_NONE;
    }
}

