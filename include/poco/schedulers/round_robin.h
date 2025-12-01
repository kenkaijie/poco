// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief A basic round-robin scheduler.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <poco/coro.h>
#include <poco/intracoro.h>
#include <poco/platform.h>
#include <poco/queue.h>
#include <poco/scheduler.h>
#include <stddef.h>

/** Maximum number of external events a scheduler can handle between each yield. */
#define SCHEDULER_MAX_EXTERNAL_EVENT_COUNT (16)

typedef struct round_robin_scheduler {
    Scheduler scheduler;
    Coro **tasks;
    size_t max_tasks_count; /**< Maximum number of tasks the task array can store. */
    size_t all_tasks;       /**<* Number of actual tasks in the task list. */
    size_t finished_tasks;
    Coro *current_task;
    size_t next_task_index; /**< index to check next when performing a context switch */
    Queue event_queue;
    CoroEventSource external_events[SCHEDULER_MAX_EXTERNAL_EVENT_COUNT];
    PlatformTicks previous_ticks;
} RoundRobinScheduler;

/*!
 * @brief Create a basic scheduler.
 *
 * @param coro_list List of coroutines to schedule.
 * @param num_coros Number of coroutines in the list.
 *
 * @return Pointer to the scheduler, or NULL on error.
 */
Scheduler *round_robin_scheduler_create(Coro *const *coro_list, size_t num_coros);

Scheduler *round_robin_scheduler_create_static(RoundRobinScheduler *scheduler,
                                               Coro **coro_list, size_t num_coros);

/*!
 * @brief Frees a dynamically allocated scheduler.
 *
 * @param scheduler Scheduler to free, must have been created from @ref
 *      round_robin_scheduler_create.
 */
void round_round_robin_scheduler_free(RoundRobinScheduler *scheduler);

/*!
 * @brief Add a coroutine to the scheduler.
 *
 * @note This will only use empty slots. If a coroutine is finished, its slow must be
 *      explicitly cleared by the caller. This is to guarantee to the caller that the
 *      scheduler holds the reference (until being told not to).
 *
 * @param scheduler
 * @param coro
 *
 * @retval #RES_OK if the coroutine has been added
 * @retval #RES_NO_MEM if there was no space for the coroutine
 */
Result round_robin_scheduler_add_coro(RoundRobinScheduler *scheduler, Coro *coro);
void round_robin_scheduler_remove_coro(RoundRobinScheduler *scheduler,
                                       Coro const *coro);

#ifdef __cplusplus
}
#endif
