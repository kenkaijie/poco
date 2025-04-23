/*!
 * @file
 * @brief Communication primitives between the coroutine and the scheduler.
 * 
 * Every scheduler implementation must uphold this interface.
 * 
 * Event sinks and sources are connected , that is, event sources can update one
 * or more event sinks.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <poco/platform.h>

/*!
 * @brief Types of signals that can be sent to the scheduler.
 *
 * Each one encoders an expected behaviour.
 */
typedef enum {
    /**
     * Coroutine is waiting for a signal, this implies the sinks have been configured.
     * Corotuine should not be resumed until any of the signals are triggered.
     */
    CORO_SIG_WAIT = 1,

    /**
     * This is a basic yield. Scheduler should place this corotuine back into the list
     * of coroutines to be scheduled, but update any blocked coroutines with the
     * provided event. Implies the event_source is active.
     */
    CORO_SIG_NOTIFY = 2,

    /**
     * Combination of both notifying and also blocking. Implies both event sources and
     * event sinks are valid.
     */
    CORO_SIG_NOTIFY_AND_WAIT = 3,

    /**
     * Special indicator indicating the corotuine is done and should no longer be
     * scheduled.
     */
    CORO_SIG_DONE = 4,   // Coroutine is done.
} coro_signal_type_t;

/*!
 * @brief Properties the coroutine communicates back to the scheduler.
 * 
 * This applies to signals that have further information to communicate.
 */
typedef struct
{
    coro_signal_type_t  type;   // Type of signal.
} coro_signal_t;

typedef enum {
    /** Disabled event. */
    CORO_EVTSINK_NONE = 0,

    /** Coroutine is waiting for a time delay before it can resume. */
    CORO_EVTSINK_DELAY,

    /** Corotuine is waiting on a queue to have an item. Uses the queue parameter. */
    CORO_EVTSINK_QUEUE_NOT_FULL,

    /** Coroutine is waiting on a queue to have space. Uses the queue parameter.*/
    CORO_EVTSINK_QUEUE_NOT_EMPTY,

    /** Coroutine is waiting on an event. Uses the event parameter. */
    CORO_EVTSINK_EVENT_GET,
} coro_event_sink_type_t;

typedef struct 
{
    coro_event_sink_type_t type;
    union {
        platform_ticks_t ticks_remaining;
        void * queue;
        void * event;
    } params;
} coro_event_sink_t;

typedef enum {
    /* No special event. */
    CORO_EVTSRC_NOOP = 0,

    /* Indicates that an elasped period of time has passed. */
    CORO_EVTSRC_ELAPSED,

    /* Indicates a queue has had an item put in it, coroutines waiting should unblock. Uses the queue parameter. */
    CORO_EVTSRC_QUEUE_PUT,

    /* Indicates a queue has had an item removed from it, coroutines waiting should unblock. Uses the queue parameter. */
    CORO_EVTSRC_QUEUE_GET,

    /* An event has one of its field set. */
    CORO_EVTSRC_EVENT_SET,
    
} coro_event_source_type_t;


typedef struct 
{
    coro_event_source_type_t type;
    union {
        platform_ticks_t elasped_ticks;
        void * queue;
        void * event;
    } params;
} coro_event_source_t;

/*!
 * @brief Update the sink with the provided event source.
 *
 * @note Some events may take multiple events to trigger, such as the time based events.
 * 
 * @param sink Sink to update.
 * @param event Event to apply.
 * 
 * @return True if the event has triggered.
 */
bool intercoro_update_event_sink(coro_event_sink_t * sink, coro_event_source_t const * event);
