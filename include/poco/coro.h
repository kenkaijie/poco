// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Declarations for coroutines in poco.
 *
 * Implements coroutines that can be suspended and resumed.
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
 * @brief Preset event slot indices.
 *
 * A 2 slot system is likely all we need here:
 *
 * 1. Primary slot for a communication primitive event.
 * 2. Optional time slot for timeouts.
 *
 * @note There shouldn't be a need for any more than what is defined.
 */
enum event_sink_slot {
    /** Primary slot reserved for the main (non-timeout) event of this yield. */
    EVENT_SINK_SLOT_PRIMARY = 0,
    /** Secondary slot reserved for timeout. */
    EVENT_SINK_SLOT_TIMEOUT,
    EVENT_SINK_SLOT_COUNT, // Must always be last.
};

/*!
 * @brief Defines the states a coroutine can be at any given time.
 */
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

typedef struct coro Coro;

/*!
 * @brief Function declaration for the coroutine entrypoint.
 *
 * @param context Provided user context when creating the coroutine.
 */
typedef void (*CoroEntrypoint)(void *context);

/*!
 * @brief Represents a coroutine that can be scheduled and executed.
 */
struct coro {

    /** Coroutine's main entrypoint function. */
    CoroEntrypoint entrypoint;

    /** Stack Declaration */
    PlatformStackElement *stack;

    /** Stack length, in bytes. Will always be an integer multiple of
     * PlatformStackElement.
     */
    size_t stack_size;

    /** The event sink item that unblocked this sink, only valid after a blocked
     * coroutine as been unblocked.
     */
    size_t triggered_event_sink_slot;

    /** Managed event source, only valid if the coroutine has notified the scheduler. */
    CoroEventSource event_source;

    /** Managed event sinks, only valid if the coroutine is blocked. */
    CoroEventSink event_sinks[EVENT_SINK_SLOT_COUNT];

    // Ping pong contexts
    PlatformContext suspend_context;
    PlatformContext resume_context;

    /** The current coroutine state. Schedulers should only have read-access to this. */
    coro_state_t coro_state;

    /** For a non-running coroutine, this is the signal it last yielded with. */
    CoroSignal yield_signal;
};

/*!
 * @brief Creates a statically defined coroutine with the specific stack and entrypoint.
 *
 * @note For stack diagnostics, we actually consume the first and last element of the
 *       stack for watermarks. The consequence is that the actual usable stack size will
 *       be 2 less than the declared value.
 *
 * @param coro Coroutine descriptor to initialise.
 * @param entrypoint Entrypoint function.
 * @param context User context passed into the entrypoint function.
 * @param stack Pointer to a predefined stack space (must be word aligned).
 * @param stack_count Number of platform specific elements in the stack.
 *
 * @return pointer to the coroutine, or NULL if parameters are invalid.
 */
Coro *coro_create_static(Coro *coro, CoroEntrypoint entrypoint, void *context,
                         PlatformStackElement *stack, size_t stack_count);

/*!
 * @brief Destroy a static coroutine.
 *
 * @note This does not free memory, it just clears the underlying platform specific
 * context members.
 *
 * @param coro Coroutine created using coro_create_static.
 */
void coro_destroy_static(Coro *coro);

/*!
 * @brief Creates a coroutine with the specific stack and entrypoint.
 *
 * @note For stack diagnostics, we actually consume the first and last element of the
 *       stack for watermarks. The consequence is that the actual usable stack size will
 *       be 2 less than the declared value.
 *
 * @param entrypoint Entrypoint function.
 * @param context User context passed into the entrypoint function.
 * @param stack_count Number of platform specific elements in the stack.
 *
 * @return pointer to the coroutine, or NULL if a coroutine cannot be created.
 */
Coro *coro_create(CoroEntrypoint entrypoint, void *context, size_t stack_count);

/*!
 * @brief Frees a dynamically created coroutine.
 *
 * @warning Using this on a statically created coroutine is undefined.
 *
 * @param coro Coroutine to free.
 */
void coro_free(Coro *coro);

/*!
 * @brief Called by the coroutine to yield control back to the scheduler.
 *
 * The coroutine will be placed immediately back into the scheduler. Depending on the
 * scheduler, this may cause it to be scheduled again immediately.
 *
 * This yields with the NONE event source.
 *
 * @note This function will block until the coroutine is resumed again.
 */
void coro_yield(void);

/*!
 * @brief Yield the coroutine with a specific delay.
 *
 * Delay times are "at least" values, the guarantee is that the coroutine will not
 * resume until at least the specified time has passed. The scheduler may resume the
 * coroutine later if required.
 *
 * @param duration_ms Minimum duration in milliseconds the coroutine should wait.
 */
void coro_yield_delay(int64_t duration_ms);

/*!
 * @brief Yield a coroutine with the provided signal source.
 *
 * This yields the coroutine with #CORO_SIG_NOTIFY.
 *
 * @warning This is a special operation typically used for scheduler or communication
 *          primitive development. User application constructs should stick with regular
 *          yields.
 *
 * @param event Event to yield.
 */
void coro_yield_with_event(CoroEventSource const *event);

/*!
 * @brief Yield a coroutine with the provided signal type.
 *
 * @warning This is a low-level yield, the caller is expected to correctly set up the
 *          coroutine's internal state before calling this to ensure correct operation.
 *
 * @param signal Signal to yield.
 */
void coro_yield_with_signal(CoroSignal signal);

/*!
 * @brief Notify a coroutine of an event that may affect it's internal state.
 *
 * @note If a coroutine is not blocked, the events are ignored.
 *
 * @warning This is a special operation typically used for scheduler or communication
 *          primitive development.
 *
 * @param coro Coroutine to notify.
 * @param event Notification event.
 *
 * @return True if the coroutine's state has changed.
 */
bool coro_notify(Coro *coro, CoroEventSource const *event);

/*!
 * @brief Resumes the coroutine from the point it last yielded.
 *
 * @warning This is a special operation typically used for scheduler or communication
 *          primitive development.
 *
 * @param coro Coroutine to resume.
 *
 * @return Signal to the scheduler to action.
 */
CoroSignal coro_resume(Coro *coro);

/*!
 * @brief Join and waits for the target coroutine to finish before resuming.
 *
 * @param coro Coroutine to join.
 */
void coro_join(Coro *coro);

#ifdef __cplusplus
}
#endif