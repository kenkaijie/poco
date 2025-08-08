// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief A basic round robin scheduler.
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
    scheduler_t scheduler;
    coro_t *const *tasks;
    size_t task_count;
    size_t finished_tasks;
    coro_t *current_task;
    size_t next_task_index; /**< index to check next when performing a context switch */
    queue_t event_queue;
    coro_event_source_t external_events[SCHEDULER_MAX_EXTERNAL_EVENT_COUNT];
    platform_ticks_t previous_ticks;
} round_robin_scheduler_t;

/*!
 * @brief Create a basic scheduler.
 *
 * @param coro_list List of coroutines to schedule.
 * @param num_coros Number of coroutines in the list.
 *
 * @return Pointer to the scheduler, or NULL on error.
 */
scheduler_t *round_robin_scheduler_create(coro_t *const *coro_list, size_t num_coros);

scheduler_t *round_robin_scheduler_create_static(round_robin_scheduler_t *scheduler,
                                                 coro_t *const *coro_list,
                                                 size_t num_coros);

#ifdef __cplusplus
}
#endif
