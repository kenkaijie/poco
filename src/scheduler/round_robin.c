// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Round robin scheduler implementation.
 */

#include <poco/platform.h>
#include <poco/schedulers/round_robin.h>
#include <string.h>

/*!
 * @brief Add to the task array's index taking into account the number of tasks.
 */
static inline size_t _increment_task_index(round_robin_scheduler_t *scheduler,
                                           size_t task_index, size_t step) {
    return (task_index + step) % scheduler->max_tasks_count;
}

static result_t _notify_from_isr(round_robin_scheduler_t *scheduler,
                                 coro_event_source_t const *event) {
    result_t queue_result = queue_raw_put(&scheduler->event_queue, (void const *)event);
    return (queue_result == RES_OK) ? RES_OK : RES_NOTIFY_FAILED;
}

static result_t _notify(round_robin_scheduler_t *scheduler,
                        coro_event_source_t const *event) {
    result_t queue_result;

    platform_enter_critical_section();
    queue_result = queue_raw_put(&scheduler->event_queue, (void const *)event);
    platform_exit_critical_section();

    return (queue_result == RES_OK) ? RES_OK : RES_NOTIFY_FAILED;
}

static coro_t *_get_current_coro(round_robin_scheduler_t *scheduler) {
    return scheduler->current_task;
}

static size_t _get_finished_task_count(coro_t *const *coro_list, size_t max_count) {
    size_t finished_tasks = 0;
    for (size_t idx = 0; idx < max_count; ++idx) {
        coro_t *task = coro_list[idx];
        if ((task != NULL) && (task->coro_state == CORO_STATE_FINISHED)) {
            finished_tasks++;
        }
    }
    return finished_tasks;
}

/*!
 * Gets the actual task count based on non-empty values in the coroutine list.
 */
static size_t _get_task_count(coro_t *const *coro_list, size_t max_count) {
    size_t coroutine_count = 0;
    for (size_t idx = 0; idx < max_count; ++idx) {
        coroutine_count += (coro_list[idx] != NULL) ? 1 : 0;
    }
    return coroutine_count;
}

static coro_t *_get_next_ready_task(round_robin_scheduler_t *scheduler) {
    for (size_t offset = 0; offset < scheduler->max_tasks_count; ++offset) {
        size_t task_index =
            _increment_task_index(scheduler, scheduler->next_task_index, offset);
        coro_t *task = scheduler->tasks[task_index];
        if ((task != NULL) && (task->coro_state == CORO_STATE_READY)) {
            scheduler->current_task = task;
            scheduler->next_task_index =
                _increment_task_index(scheduler, task_index, 1);
            return scheduler->current_task;
        }
    }
    return NULL;
}

/*!
 * @brief Update waiting tasks with the event.
 */
static void _update_waiting_tasks(round_robin_scheduler_t *scheduler,
                                  coro_event_source_t const *event) {
    for (size_t task_idx = 0; task_idx < scheduler->max_tasks_count; ++task_idx) {
        coro_t *task = scheduler->tasks[task_idx];
        if (task != NULL) {
            coro_notify(task, event);
        }
    }
}

static void _scheduler_start(round_robin_scheduler_t *scheduler) {
    scheduler->finished_tasks =
        _get_finished_task_count(scheduler->tasks, scheduler->max_tasks_count);
    scheduler->previous_ticks = platform_get_monotonic_ticks();
}

static bool _scheduler_run_once(round_robin_scheduler_t *scheduler) {
    if (scheduler->finished_tasks >= scheduler->all_tasks) {
        /* no more tasks to run */
        return false;
    }

    coro_t *next_coro = _get_next_ready_task(scheduler);
    // If there are no tasks to run, we can just wait for the next event.
    if (next_coro != NULL) {
        coro_signal_t signal = coro_resume(next_coro);

        coro_event_source_t *coroutine_event = NULL;

        switch (signal) {
        case CORO_SIG_NOTIFY_AND_DONE:
            scheduler->finished_tasks++;
            /* FALLTHRU */
        case CORO_SIG_NOTIFY:
            /* FALLTHRU */
        case CORO_SIG_NOTIFY_AND_WAIT:
            coroutine_event = &next_coro->event_source;
            break;
        case CORO_SIG_WAIT:
            // do nothing, unblock if an event is triggered.
            break;
        }
        if (coroutine_event != NULL) {
            _update_waiting_tasks(scheduler, coroutine_event);
        }
    }

    platform_ticks_t current_ticks = platform_get_monotonic_ticks();

    // Process a time signal, which is synthesized within the scheduler.
    if (current_ticks != scheduler->previous_ticks) {
        coro_event_source_t time_event = {
            .type = CORO_EVTSRC_ELAPSED,
            .params.elasped_ticks = (current_ticks - scheduler->previous_ticks),
        };
        scheduler_notify((scheduler_t *)scheduler, &time_event);
        scheduler->previous_ticks = current_ticks;
    }

    // dequeue all items in the external event queue
    size_t external_event_count = queue_item_count(&scheduler->event_queue);
    for (size_t i = 0; i < external_event_count; ++i) {
        coro_event_source_t event;
        if (queue_raw_get(&scheduler->event_queue, (void *)&event) == RES_OK) {
            _update_waiting_tasks(scheduler, &event);
        }
    }

    return true;
}

static void _scheduler_loop(round_robin_scheduler_t *scheduler) {
    bool keep_running = true;
    _scheduler_start(scheduler);
    while (keep_running) {
        keep_running = _scheduler_run_once(scheduler);
    }
}

scheduler_t *round_robin_scheduler_create(coro_t *const *coro_list, size_t num_coros) {
    round_robin_scheduler_t *scheduler = malloc(sizeof(round_robin_scheduler_t));
    coro_t **copied_list = (coro_t **)malloc(num_coros * sizeof(coro_t *));

    if (scheduler == NULL || coro_list == NULL) {
        /* No more memory. */
        return NULL;
    }

    for (size_t i = 0; i < num_coros; ++i) {
        copied_list[i] = coro_list[i];
    }

    return round_robin_scheduler_create_static(scheduler, copied_list, num_coros);
}

scheduler_t *round_robin_scheduler_create_static(round_robin_scheduler_t *scheduler,
                                                 coro_t **coro_list, size_t num_coros) {
    scheduler->scheduler.start = (scheduler_start_t)_scheduler_start;
    scheduler->scheduler.run_once = (scheduler_run_once_t)_scheduler_run_once;
    scheduler->scheduler.run = (scheduler_run_t)_scheduler_loop;
    scheduler->scheduler.notify_from_isr =
        (scheduler_notify_from_isr_t)_notify_from_isr;
    scheduler->scheduler.notify = (scheduler_notify_t)_notify;
    scheduler->scheduler.get_current_coroutine =
        (scheduler_get_current_coroutine_t)_get_current_coro;
    scheduler->tasks = coro_list;
    scheduler->max_tasks_count = num_coros;
    scheduler->all_tasks = _get_task_count(coro_list, num_coros);
    scheduler->finished_tasks = 0;
    scheduler->current_task = NULL;
    scheduler->next_task_index = 0;

    memset(scheduler->external_events, 0, sizeof(scheduler->external_events));
    queue_create_static(&scheduler->event_queue, SCHEDULER_MAX_EXTERNAL_EVENT_COUNT,
                        sizeof(coro_event_source_t),
                        (uint8_t *)scheduler->external_events);

    return (scheduler_t *)scheduler;
}

void round_round_robin_scheduler_free(round_robin_scheduler_t *scheduler) {
    if (scheduler == NULL) {
        /* cannot free null pointer, as we cannot access the stack to free it first. */
        return;
    }

    if (scheduler->tasks != NULL) {
        /* Note we dont free the underlying coroutines, just the scheduler! */
        free((coro_t **)scheduler->tasks);
    }

    free(scheduler);
}

result_t round_robin_scheduler_add_coro(round_robin_scheduler_t *scheduler,
                                        coro_t *coro) {
    // This doens't have to be particularly fast, a linear search is sufficient.

    for (size_t idx = 0; idx < scheduler->max_tasks_count; ++idx) {
        coro_t *task = scheduler->tasks[idx];

        if (task == NULL) {
            scheduler->tasks[idx] = coro;
            scheduler->all_tasks =
                _get_task_count(scheduler->tasks, scheduler->max_tasks_count);
            return RES_OK;
        }
    }

    return RES_NO_MEM;
}

void round_robin_scheduler_remove_coro(round_robin_scheduler_t *scheduler,
                                       coro_t *coro) {
    // This doens't have to be particularly fast, a linear search is sufficient.

    for (size_t idx = 0; idx < scheduler->max_tasks_count; ++idx) {
        coro_t *task = scheduler->tasks[idx];

        if ((task == coro)) {
            scheduler->tasks[idx] = NULL;
            scheduler->all_tasks =
                _get_task_count(scheduler->tasks, scheduler->max_tasks_count);
            return;
        }
    }
}
