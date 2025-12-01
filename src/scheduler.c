// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Implementation for scheduler glue.
 */

#include <poco/context.h>
#include <poco/scheduler.h>

void scheduler_run(Scheduler *scheduler) {
    context_set_scheduler(scheduler);
    scheduler->run(scheduler);
}
