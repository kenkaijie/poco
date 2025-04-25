/*!
 * @file
 * @brief Coroutines.
 *
 * Implements coroutines that can be suspended and resumed.
 *
 * This file provides 2 yielding modes:
 * - Yielding without delay (immediate).
 * - Yielding with delay (at least).
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <poco/intracoro.h>
#include <poco/platform.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/*!
 * Event slot declaration.
 *
 * There shouldn't be a need for any more than what is defined.
 *
 * 1. Primary slot for a communication primitive event.
 * 2. Optional time slot for timeouts.
 */
enum event_sink_slot {
    /** Primary slot reserved for the main (non-timeout) event of this yield. */
    EVENT_SINK_SLOT_PRIMARY = 0,
    /** Secondary slot reserved for timeout. */
    EVENT_SINK_SLOT_TIMEOUT,
    EVENT_SINK_SLOT_COUNT, // Must always be last.
};

typedef enum coro_state {
    /* Coro is waiting to be scheduled. */
    CORO_STATE_READY = 0,
    /* Coro is the currently running one. */
    CORO_STATE_RUNNING = 1,
    /* Coro has yielded and is waiting for one of the configured triggers to resume. */
    CORO_STATE_BLOCKED = 2,
    /* Coro has finished. */
    CORO_STATE_FINISHED = 3,
} coro_state_t;

typedef struct coro coro_t;
typedef void (*coro_function_t)(coro_t *coro, void *context);

/*!
 * @brief Represents a coroutine that can be scheduled and executed.
 */
struct coro {
    /** The current corotuine state. Schedulers should only has read-access to this. */
    coro_state_t coro_state;

    /** Coroutine's main entrypoint function. */
    coro_function_t entrypoint;

    // Ping pong contexts
    platform_context_t suspend_context;
    platform_context_t resume_context;

    /** For a non running coroutine, this is the signal it last yielded with. */
    coro_signal_t yield_signal;

    /** Managed event source, only valid if the coroutine has notified the scheduler. */
    coro_event_source_t event_source;

    /** Managed event sinks, only valid if the coroutine is blocked. */
    coro_event_sink_t event_sinks[EVENT_SINK_SLOT_COUNT];
};

/*!
 * @brief Macro to define a static coroutine with a stack of a specific size.
 *
 * Ensures proper alignment of the structures. Normal use should avoid using
 * the variables directly, instead should use the pointer returned from
 * coro_create_static().
 *
 * @param name Name of the coroutine.
 * @param stack_size_words Size of the stack, in words.
 */
#define CORO_STATIC_DEFINE(name, stack_size_words)                                     \
    static platform_stack_t name##_stack[stack_size_words];  \
    static coro_t name##_coro;

/*!
 * @brief Creates a coroutine with the specific stack and entrypoint.
 *
 * @param coro Coroutine descriptor to initialise.
 * @param function Entrypoint function.
 * @param context User context passed into the entrypoint function.
 * @param stack Pointer to a predefined stack space (must be word aligned).
 * @param stack_size Size of the stack, in bytes.
 */
coro_t *coro_create_static(coro_t *coro, coro_function_t function, void *context,
                           uint32_t *stack, size_t stack_size);

/*!
 * @brief Called by the coroutine to yield control back to the scheduler.
 *
 * The coroutine will be placed immediately back into the scheduler. Depending on the
 * scheduler, this may cause it to be scheduled again immediately.
 *
 * This yields with the NONE event source.
 *
 * @note This function will block until the coroutine is resumed again.
 *
 * @param coro Coroutine to yield.
 */
void coro_yield(coro_t *coro);

/*!
 * @brief Yield the coroutine with a specific delay.
 *
 * Delay times are "at least" values, the guarantee is that the coroutine will not
 * resume until at least the specified time has passed. The scheduler may resume the
 * coroutine later if required.
 *
 * @param coro Coroutine to yield.
 * @param delay Minimum duration in milliseconds the coroutine should wait.
 */
void coro_yield_delay(coro_t *coro, int64_t delay);

/* These are special operations used internally in awaitable primitives. These
 * should not be required, unless you are creating a custom scheduler.
 */

void coro_yield_with_event(coro_t *coro, coro_event_source_t const *event);
void coro_yield_with_signal(coro_t *coro, coro_signal_type_t signal);

/*!
 * @brief Notify a coroutine of an event that may affect it's internal state.
 *
 * @note If a coroutine is not blocked, the events are ignored.
 *
 * @param coro Coroutine to notify.
 * @param event Notification event.
 *
 * @return True if the coroutine's state has changed.
 */
bool coro_notify(coro_t *coro, coro_event_source_t const *event);

coro_signal_type_t coro_resume(coro_t *coro);

#ifdef __cplusplus
}
#endif