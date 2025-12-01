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
static size_t increment_task_index(RoundRobinScheduler const *scheduler,
                                   size_t const task_index, size_t const step) {
    return (task_index + step) % scheduler->max_tasks_count;
}

static Result notify_from_isr(RoundRobinScheduler *scheduler,
                              CoroEventSource const *event) {
    Result const queue_result = queue_raw_put(&scheduler->event_queue, event);
    return (queue_result == RES_OK) ? RES_OK : RES_NOTIFY_FAILED;
}

static Result notify(RoundRobinScheduler *scheduler, CoroEventSource const *event) {

    platform_enter_critical_section();
    Result const queue_result = queue_raw_put(&scheduler->event_queue, event);
    platform_exit_critical_section();

    return (queue_result == RES_OK) ? RES_OK : RES_NOTIFY_FAILED;
}

static Coro *get_current_coro(RoundRobinScheduler const *scheduler) {
    return scheduler->current_task;
}

static size_t get_finished_task_count(Coro *const *coro_list, size_t const max_count) {
    size_t finished_tasks = 0;
    for (size_t idx = 0; idx < max_count; ++idx) {
        Coro const *task = coro_list[idx];
        if ((task != NULL) && (task->coro_state == CORO_STATE_FINISHED)) {
            finished_tasks++;
        }
    }
    return finished_tasks;
}

/*!
 * Gets the actual task count based on non-empty values in the coroutine list.
 */
static size_t get_task_count(Coro *const *coro_list, size_t const max_count) {
    size_t coroutine_count = 0;
    for (size_t idx = 0; idx < max_count; ++idx) {
        coroutine_count += (coro_list[idx] != NULL) ? 1 : 0;
    }
    return coroutine_count;
}

static Coro *get_next_ready_task(RoundRobinScheduler *scheduler) {
    for (size_t offset = 0; offset < scheduler->max_tasks_count; ++offset) {
        size_t const task_index =
            increment_task_index(scheduler, scheduler->next_task_index, offset);
        Coro *task = scheduler->tasks[task_index];
        if ((task != NULL) && (task->coro_state == CORO_STATE_READY)) {
            scheduler->current_task = task;
            scheduler->next_task_index = increment_task_index(scheduler, task_index, 1);
            return scheduler->current_task;
        }
    }
    return NULL;
}

/*!
 * @brief Update waiting tasks with the event.
 */
static void update_waiting_tasks(RoundRobinScheduler const *scheduler,
                                 CoroEventSource const *event) {
    for (size_t task_idx = 0; task_idx < scheduler->max_tasks_count; ++task_idx) {
        Coro *task = scheduler->tasks[task_idx];
        if (task != NULL) {
            coro_notify(task, event);
        }
    }
}

static void start_scheduler(RoundRobinScheduler *scheduler) {
    scheduler->finished_tasks =
        get_finished_task_count(scheduler->tasks, scheduler->max_tasks_count);
    scheduler->previous_ticks = platform_get_monotonic_ticks();
}

static bool run_scheduler_once(RoundRobinScheduler *scheduler) {
    if (scheduler->finished_tasks >= scheduler->all_tasks) {
        /* no more tasks to run */
        return false;
    }

    Coro *next_coro = get_next_ready_task(scheduler);
    // If there are no tasks to run, we can just wait for the next event.
    if (next_coro != NULL) {
        CoroSignal const signal = coro_resume(next_coro);

        CoroEventSource const *coroutine_event = NULL;

        switch (signal) {
        case CORO_SIG_NOTIFY_AND_DONE:
            scheduler->finished_tasks++;
            /* FALLTHROUGH */
        case CORO_SIG_NOTIFY:
            /* FALLTHROUGH */
        case CORO_SIG_NOTIFY_AND_WAIT:
            coroutine_event = &next_coro->event_source;
            break;
        case CORO_SIG_WAIT:
            // do nothing, unblock if an event is triggered.
            break;
        }
        if (coroutine_event != NULL) {
            update_waiting_tasks(scheduler, coroutine_event);
        }
    }

    PlatformTicks const current_ticks = platform_get_monotonic_ticks();

    // Process a time signal, which is synthesized within the scheduler.
    if (current_ticks != scheduler->previous_ticks) {
        CoroEventSource const time_event = {
            .type = CORO_EVTSRC_ELAPSED,
            .params.elapsed_ticks = (current_ticks - scheduler->previous_ticks),
        };
        scheduler_notify((Scheduler *)scheduler, &time_event);
        scheduler->previous_ticks = current_ticks;
    }

    // dequeue all items in the external event queue
    size_t const external_event_count = queue_item_count(&scheduler->event_queue);
    for (size_t i = 0; i < external_event_count; ++i) {
        CoroEventSource event;
        if (queue_raw_get(&scheduler->event_queue, (void *)&event) == RES_OK) {
            update_waiting_tasks(scheduler, &event);
        }
    }

    return true;
}

static void run_scheduling_loop(RoundRobinScheduler *scheduler) {
    bool keep_running = true;
    start_scheduler(scheduler);
    while (keep_running) {
        keep_running = run_scheduler_once(scheduler);
    }
}

Scheduler *round_robin_scheduler_create(Coro *const *coro_list,
                                        size_t const num_coros) {
    RoundRobinScheduler *scheduler = malloc(sizeof(RoundRobinScheduler));

    if (scheduler == NULL) {
        /* No more memory. */
        return NULL;
    }

    Coro **copied_list = malloc(num_coros * sizeof(Coro *));

    if (copied_list == NULL) {
        /* No more memory. */
        free(scheduler);
        return NULL;
    }

    for (size_t i = 0; i < num_coros; ++i) {
        copied_list[i] = coro_list[i];
    }

    return round_robin_scheduler_create_static(scheduler, copied_list, num_coros);
}

Scheduler *round_robin_scheduler_create_static(RoundRobinScheduler *scheduler,
                                               Coro **coro_list,
                                               size_t const num_coros) {
    scheduler->scheduler.run = (scheduler_run_t)run_scheduling_loop;
    scheduler->scheduler.notify_from_isr = (scheduler_notify_from_isr_t)notify_from_isr;
    scheduler->scheduler.notify = (scheduler_notify_t)notify;
    scheduler->scheduler.get_current_coroutine =
        (scheduler_get_current_coroutine_t)get_current_coro;
    scheduler->tasks = coro_list;
    scheduler->max_tasks_count = num_coros;
    scheduler->all_tasks = get_task_count(coro_list, num_coros);
    scheduler->finished_tasks = 0;
    scheduler->current_task = NULL;
    scheduler->next_task_index = 0;

    memset(scheduler->external_events, 0, sizeof(scheduler->external_events));
    queue_create_static(&scheduler->event_queue, SCHEDULER_MAX_EXTERNAL_EVENT_COUNT,
                        sizeof(CoroEventSource), (uint8_t *)scheduler->external_events);

    return (Scheduler *)scheduler;
}

void round_round_robin_scheduler_free(RoundRobinScheduler *scheduler) {
    if (scheduler == NULL) {
        /* Cannot free null pointer, as we cannot access the stack to free it first. */
        return;
    }

    if (scheduler->tasks != NULL) {
        /* Note we don't free the underlying coroutines, just the scheduler! */
        free(scheduler->tasks);
    }

    free(scheduler);
}

Result round_robin_scheduler_add_coro(RoundRobinScheduler *scheduler, Coro *coro) {
    // This doesn't have to be particularly fast, a linear search is sufficient.

    for (size_t idx = 0; idx < scheduler->max_tasks_count; ++idx) {
        Coro const *task = scheduler->tasks[idx];

        if (task == NULL) {
            scheduler->tasks[idx] = coro;
            scheduler->all_tasks =
                get_task_count(scheduler->tasks, scheduler->max_tasks_count);
            return RES_OK;
        }
    }

    return RES_NO_MEM;
}

void round_robin_scheduler_remove_coro(RoundRobinScheduler *scheduler,
                                       Coro const *coro) {
    // This doesn't have to be particularly fast, a linear search is sufficient.

    for (size_t idx = 0; idx < scheduler->max_tasks_count; ++idx) {
        Coro const *task = scheduler->tasks[idx];

        if (task == coro) {
            scheduler->tasks[idx] = NULL;
            scheduler->all_tasks =
                get_task_count(scheduler->tasks, scheduler->max_tasks_count);
            return;
        }
    }
}
