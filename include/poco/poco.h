/*!
 * @file
 * @brief Aggregate include for the entire poco library.
 *
 * SPDX-FileCopyrightText: Copyright contributors to the poco project.
 * SPDX-License-Identifier: MIT
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <poco/context.h>
#include <poco/coro.h>
#include <poco/event.h>
#include <poco/intracoro.h>
#include <poco/queue.h>
#include <poco/result.h>
#include <poco/scheduler.h>
#include <poco/semaphore.h>

/* Also include all the known schedulers. */
#include <poco/schedulers/round_robin.h>

#ifdef __cplusplus
}
#endif