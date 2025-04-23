/*!
 * @file
 * @brief Scheduler Common interface, used by all schedulers.
 * 
 * In order to utilise this interface, the first member of any scheduler should be
 * the vtable.
 */

#pragma once

#include <poco/error.h>
#include <poco/intercoro.h>

typedef struct scheduler scheduler_t;

/*!
 * @brief Function pointer implementing the run function for a scheduler.
 */
typedef void (*scheduler_run_t)(scheduler_t * scheduler);

/*!
 * @brief Function pointer implementing the notify the scheduler of an event.
 * 
 * @note This should not generally be used by the user application. The events the
 *       scheduler handles are primarily for internal use.
 * 
 * @param scheduler Scheduler to notify.
 * @param event Event to notify.
 * 
 * @retval RET_OK No error.
 * @retval RET_NO_MEM No space available in the scheduler to queue this event.
 */
typedef error_t (*scheduler_notify_t)(scheduler_t * scheduler, coro_event_source_t const * event);


/*!
 * @brief Function pointer implementing the notify from ISR function.
 *
 * This is primarily used in ISR contexts, where communication primitives will
 * send their event sources directly to the scheduler, instead of via the coroutine.
 * 
 * @param scheduler Scheduler to notify.
 * @param event Event to notify.
 * 
 * @retval RET_OK No error.
 * @retval RET_NO_MEM No space available in the scheduler to queue this event.
 */
typedef error_t (*scheduler_notify_from_isr_t)(scheduler_t * scheduler, coro_event_source_t const * event);


typedef struct scheduler
{
    scheduler_run_t run;
    scheduler_notify_t notify;
    scheduler_notify_from_isr_t notify_from_isr;
} scheduler_t;

/*!
 * @brief Runs the scheduler until completion.
 *
 * A scheduler will be considered complete when all its managed coroutines are run until
 * completion.
 * 
 * If there are corotuines that never finish, run will also never finish.
 * 
 * @param scheduler Scheduler to run.
 */
__attribute__((always_inline))
static inline void scheduler_run(scheduler_t * scheduler)
{
    return scheduler->run(scheduler);
}

__attribute__((always_inline))
static inline error_t scheduler_notify(scheduler_t * scheduler, coro_event_source_t const * event)
{
    return scheduler->notify(scheduler, event);
}


__attribute__((always_inline))
static inline error_t scheduler_notify_from_isr(scheduler_t * scheduler, coro_event_source_t const * event)
{
    return scheduler->notify_from_isr(scheduler, event);
}
