// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Context implementation for single threaded scheduler support.
 */

#include <poco/context.h>

static Scheduler *assigned_scheduler = NULL;

Scheduler *context_get_scheduler(void) { return assigned_scheduler; }

Coro *context_get_coro(void) {
    return scheduler_get_current_coroutine(assigned_scheduler);
}

void context_set_scheduler(Scheduler *scheduler) { assigned_scheduler = scheduler; }
