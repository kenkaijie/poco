// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Declarations for special coroutine operations.
 *
 * Most of these functions are either used in scheduler development, or in very special
 * cases.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <poco/coro.h>

/*!
 * @brief Yield a coroutine with the provided signal source.
 *
 * This yields the coroutine with #CORO_SIG_NOTIFY.
 *
 * @warning This is a special operation typically used for scheduler or communication
 *          primitive development. User application constructs should stick with regular
 *          yields.
 *
 * @param event Event to yield.
 */
void coro_yield_with_event(CoroEventSource const *event);

/*!
 * @brief Yield a coroutine with the provided signal type.
 *
 * @warning This is a low-level yield, the caller is expected to correctly set up the
 *          coroutine's internal state before calling this to ensure correct operation.
 *
 * @param signal Signal to yield.
 */
void coro_yield_with_signal(CoroSignal signal);

/*!
 * @brief Notify a coroutine of an event that may affect it's internal state.
 *
 * @note If a coroutine is not blocked, the events are ignored.
 *
 * @warning This is a special operation typically used for scheduler or communication
 *          primitive development.
 *
 * @param coro Coroutine to notify.
 * @param event Notification event.
 *
 * @return True if the coroutine's state has changed.
 */
bool coro_notify(Coro *coro, CoroEventSource const *event);

/*!
 * @brief Resumes the coroutine from the point it last yielded.
 *
 * @warning This is a special operation typically used for scheduler or communication
 *          primitive development.
 *
 * @param coro Coroutine to resume.
 *
 * @return Signal to the scheduler to action.
 */
CoroSignal coro_resume(Coro *coro);

#ifdef __cplusplus
}
#endif