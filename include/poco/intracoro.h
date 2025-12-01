// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Communication primitives between the coroutine and the scheduler.
 *
 * Event sinks and sources are connected that is, event sources can update one
 * or more event sinks.
 */

#pragma once

#include <poco/platform.h>

/*!
 * @brief Types of signals that can be sent to the scheduler.
 *
 * Each one encodes an expected behaviour.
 */
typedef enum coro_signal {
    /**
     * Coroutine is waiting for a signal, this implies the sinks have been configured.
     * coroutine should not be resumed until any of the signals are triggered.
     */
    CORO_SIG_WAIT = 0,

    /**
     * This is a basic yield. Scheduler should place this coroutine back into the list
     * of coroutines to be scheduled, but update any blocked coroutines with the
     * provided event. Implies the event_source is active.
     */
    CORO_SIG_NOTIFY,

    /**
     * Combination of both notifying and also blocking. Implies both event sources and
     * event sinks are valid.
     */
    CORO_SIG_NOTIFY_AND_WAIT,

    /**
     * Special indicator indicating the coroutine is done and should no longer be
     * scheduled. Implies the event_source is active.
     */
    CORO_SIG_NOTIFY_AND_DONE,
} CoroSignal;

typedef enum coro_event_sink_type {
    /** Disabled event. */
    CORO_EVTSINK_NONE = 0,

    /** Coroutine is waiting for a time delay before it can resume. */
    CORO_EVTSINK_DELAY,

    /** Coroutine is waiting on a queue to have an item. Uses the queue parameter. */
    CORO_EVTSINK_QUEUE_NOT_FULL,

    /** Coroutine is waiting on a queue to have space. Uses the queue parameter.*/
    CORO_EVTSINK_QUEUE_NOT_EMPTY,

    /** Coroutine is waiting on an event. Uses the event parameter. */
    CORO_EVTSINK_EVENT_GET,

    /** Coroutine is waiting on a semaphore to have space. */
    CORO_EVTSINK_SEMAPHORE_ACQUIRE,

    /** Coroutine is waiting on a mutex to be released. */
    CORO_EVTSINK_MUTEX_ACQUIRE,

    /** Coroutine is waiting on another coroutine to finish. */
    CORO_EVTSINK_WAIT_FINISH,

    /** Coroutine is waiting for the stream to free up. */
    CORO_EVTSINK_STREAM_NOT_FULL,

    /** Coroutine is waiting for the stream to have some bytes. */
    CORO_EVTSINK_STREAM_NOT_EMPTY,

} CoroEventSinkType;

typedef struct coro_event_sink {
    CoroEventSinkType type;
    union {
        PlatformTicks ticks_remaining;
        void *subject;
    } params;
} CoroEventSink;

typedef enum coro_event_source_type {
    /** No special event. */
    CORO_EVTSRC_NOOP = 0,

    /** Indicates that an elapsed period of time has passed. */
    CORO_EVTSRC_ELAPSED,

    /** Indicates a queue has had an item put in it, coroutines waiting should unblock.
       Uses the queue parameter. */
    CORO_EVTSRC_QUEUE_PUT,

    /** Indicates a queue has had an item removed from it, coroutines waiting should
       unblock. Uses the queue parameter. */
    CORO_EVTSRC_QUEUE_GET,

    /** An event has one of its field set. */
    CORO_EVTSRC_EVENT_SET,

    /** A semaphore has been released. */
    CORO_EVTSRC_SEMAPHORE_RELEASE,

    /** A mutex has been released. */
    CORO_EVTSRC_MUTEX_RELEASE,

    /** Coroutine has finished. */
    CORO_EVTSRC_CORO_FINISHED,

    /** Indicate the consumer has read some bytes from the stream. */
    CORO_EVTSRC_STREAM_RECV,

    /** Indicates the producer has written some bytes to the stream. */
    CORO_EVTSRC_STREAM_SEND,

} CoroEventSourceType;

typedef struct coro_event_source {
    CoroEventSourceType type;
    union {
        PlatformTicks elapsed_ticks;
        void *subject;
    } params;
} CoroEventSource;
