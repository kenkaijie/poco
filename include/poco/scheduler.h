// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Scheduler Common interface, used by all schedulers.
 *
 * In order to utilise this interface, the first member of any scheduler should be
 * the vtable.
 *
 * Every scheduler has 2 primary functions:
 *
 * 1. Run each coroutine to completion (via resumes).
 * 2. Route coroutine signals to other coroutines.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <poco/coro.h>
#include <poco/intracoro.h>
#include <poco/result.h>

typedef struct scheduler Scheduler;

/*!
 * @brief Function prototype for running the scheduler until completion.
 *
 * A scheduler will be considered complete when all its managed coroutines are run until
 * completion.
 *
 * If there are coroutines that never finish, run will also never finish.
 *
 * @param scheduler Scheduler to run.
 */
typedef void (*SchedulerRun)(Scheduler *scheduler);

/*!
 * @brief Function prototype for preparing the scheduler for step-by-step run mode.
 *
 * This should only be called once per scheduler. Preferably just before running.
 *
 * @warning This API is used in particular scenarios and is not recommended. Consider
 * using @ref scheduler_run instead.
 *
 * @param scheduler Scheduler to start.
 */
typedef void (*SchedulerStart)(Scheduler *scheduler);

/*!
 * @brief Function prototype for step-by-step run mode.
 *
 * @warning This API is used in particular scenarios and is not recommended. Consider
 * using @ref scheduler_run instead.
 *
 * @param scheduler Scheduler to run.
 *
 * @return True if there is more potential work to do, else false.
 */
typedef bool (*SchedulerRunOnce)(Scheduler *scheduler);

/*!
 * @brief Function pointer implementing the notify the scheduler of an event.
 *
 * @note This should not generally be used by the user application. The events the
 *       scheduler handles are primarily for internal use.
 *
 * @param scheduler Scheduler to notify.
 * @param event Event to notify.
 *
 * @retval RES_OK No error.
 * @retval RES_NO_MEM No space available in the scheduler to queue this event.
 */
typedef Result (*SchedulerNotify)(Scheduler *scheduler, CoroEventSource const *event);

/*!
 * @brief Function pointer implementing to notify the scheduler from an ISR.
 *
 * This is primarily used in ISR contexts, where communication primitives will
 * send their event sources directly to the scheduler, instead of via the coroutine.
 *
 * @param scheduler Scheduler to notify.
 * @param event Event to notify.
 *
 * @retval RES_OK No error.
 * @retval RES_NO_MEM No space available in the scheduler to queue this event.
 */
typedef Result (*SchedulerNotifyFromISR)(Scheduler *scheduler,
                                         CoroEventSource const *event);

typedef Coro *(*SchedulerGetCurrentCoroutine)(Scheduler *scheduler);

/*!
 * @brief Scheduler common interface.
 */
struct scheduler {
    SchedulerRun run;
    SchedulerNotify notify;
    SchedulerNotifyFromISR notify_from_isr;
    SchedulerGetCurrentCoroutine get_current_coroutine;
};

/*!
 * @brief Runs the scheduler until completion.
 *
 * A scheduler will be considered complete when all its managed coroutines are run until
 * completion.
 *
 * If there are coroutines that never finish, run will also never finish.
 *
 * @param scheduler Scheduler to run.
 */
void scheduler_run(Scheduler *scheduler);

/*!
 * @brief Prepares the scheduler for step-by-step run mode.
 *
 * This should only be called once per scheduler. Preferably just before running.
 *
 * @warning This API is used in particular scenarios and is not recommended. Consider
 * using @ref scheduler_run instead.
 *
 * @param scheduler Scheduler to start.
 */
void scheduler_start(Scheduler *scheduler);

/*!
 * @brief Runs a single step (as part of step-by-step run mode)
 *
 * @warning This API is used in particular scenarios and is not recommended. Consider
 * using @ref scheduler_run instead.
 *
 * @param scheduler Scheduler to run.
 *
 * @return True if there is more potential work to do, else false.
 */
bool scheduler_run_once(Scheduler *scheduler);

/*!
 * @brief Notify the scheduler of an event
 *
 * This is not typically used, as the coroutines have an internal mechanism to
 * raise events with the scheduler.
 *
 * @param scheduler Scheduler to notify.
 * @param event Event to notify.
 *
 * @note other result codes are possible, but will depend on the implementation.
 *
 * @retval #RES_OK scheduler has been notified.
 * @retval #RES_NOTIFY_FAILED if the scheduler has not been notified.
 */
static inline Result scheduler_notify(Scheduler *scheduler,
                                      CoroEventSource const *event) {
    return scheduler->notify(scheduler, event);
}

/*!
 * @brief Notify the scheduler of an event from an ISR.
 *
 * @param scheduler Scheduler to notify.
 * @param event Event to notify.
 *
 * @note other result codes are possible, but will depend on the implementation.
 *
 * @retval #RES_OK scheduler has been notified.
 * @retval #RES_NOTIFY_FAILED if the scheduler has not been notified.
 */
static inline Result scheduler_notify_from_isr(Scheduler *scheduler,
                                               CoroEventSource const *event) {
    return scheduler->notify_from_isr(scheduler, event);
}

/*!
 * @brief Gets the current running coroutine.
 *
 * @param scheduler Scheduler to check.
 *
 * @retval pointer to the coroutine, or NULL if the scheduler is not running.
 */
static inline Coro *scheduler_get_current_coroutine(Scheduler *scheduler) {
    return scheduler->get_current_coroutine(scheduler);
}

#ifdef __cplusplus
}
#endif
