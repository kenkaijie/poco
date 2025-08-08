/*!
 * @file
 * @brief Implementation for scheduler glue.
 *
 * SPDX-FileCopyrightText: Copyright contributors to the poco project.
 * SPDX-License-Identifier: MIT
 */

#include <poco/context.h>
#include <poco/scheduler.h>

void scheduler_run(scheduler_t *scheduler) {
    context_set_scheduler(scheduler);
    return scheduler->run(scheduler);
}

void scheduler_start(scheduler_t *scheduler) {
    context_set_scheduler(scheduler);
    return scheduler->start(scheduler);
}

bool scheduler_run_once(scheduler_t *scheduler) {
    return scheduler->run_once(scheduler);
}
