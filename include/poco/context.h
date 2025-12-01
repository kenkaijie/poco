// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Used to store context used within the library.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <poco/coro.h>
#include <poco/scheduler.h>

/*!
 * @brief Get the scheduler.
 */
Scheduler *context_get_scheduler(void);

Coro *context_get_coro(void);

/*!
 * @brief Sets the scheduler for this context.
 *
 * @param scheduler Scheduler to set.
 */
void context_set_scheduler(Scheduler *scheduler);

#ifdef __cplusplus
}
#endif
