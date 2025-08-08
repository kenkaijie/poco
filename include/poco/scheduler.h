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

#include <poco/coro.h>
#include <poco/intracoro.h>
#include <poco/result.h>

typedef struct scheduler scheduler_t;

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
typedef void (*scheduler_run_t)(scheduler_t *scheduler);

/*!
 * @brief Function prototype for preparing the scheduler for step-by-step run mode.
 *
 * This should only be called once per scheduler. Preferrably just before running.
 *
 * @warning This API is used in particular scenarios and is not recommended. Consider
 * using @ref scheduler_run instead.
 *
 * @param scheduler Scheduler to start.
 */
typedef void (*scheduler_start_t)(scheduler_t *scheduler);

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
typedef bool (*scheduler_run_once_t)(scheduler_t *scheduler);

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
typedef result_t (*scheduler_notify_t)(scheduler_t *scheduler,
                                       coro_event_source_t const *event);

/*!
 * @brief Function pointer implementing the notify from ISR function.
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
typedef result_t (*scheduler_notify_from_isr_t)(scheduler_t *scheduler,
                                                coro_event_source_t const *event);

typedef coro_t *(*scheduler_get_current_coroutine_t)(scheduler_t *scheduler);

/*!
 * @brief Scheduler common interface.
 */
typedef struct scheduler {
    scheduler_run_t run;
    scheduler_start_t start;
    scheduler_run_once_t run_once;
    scheduler_notify_t notify;
    scheduler_notify_from_isr_t notify_from_isr;
    scheduler_get_current_coroutine_t get_current_coroutine;
} scheduler_t;

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
void scheduler_run(scheduler_t *scheduler);

/*!
 * @brief Prepares the scheduler for step-by-step run mode.
 *
 * This should only be called once per scheduler. Preferrably just before running.
 *
 * @warning This API is used in particular scenarios and is not recommended. Consider
 * using @ref scheduler_run instead.
 *
 * @param scheduler Scheduler to start.
 */
void scheduler_start(scheduler_t *scheduler);

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
bool scheduler_run_once(scheduler_t *scheduler);

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
 */
__attribute__((always_inline)) static inline result_t
scheduler_notify(scheduler_t *scheduler, coro_event_source_t const *event) {
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
 */
__attribute__((always_inline)) static inline result_t
scheduler_notify_from_isr(scheduler_t *scheduler, coro_event_source_t const *event) {
    return scheduler->notify_from_isr(scheduler, event);
}

/*!
 * @brief Gets the current running coroutine.
 *
 * @param scheduler Scheduler to check.
 *
 * @retval pointer to the coroutine, or NULL if the scheduler is not running.
 */
__attribute__((always_inline)) static inline coro_t *
scheduler_get_current_coroutine(scheduler_t *scheduler) {
    return scheduler->get_current_coroutine(scheduler);
}
