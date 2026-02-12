// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Custom observable primitive to demonstrate using a user defined primitive.
 *
 * Note this is an example only. If an a similar object is required, it should be
 * implemented using events instead, where a "target reached" event is emitted when
 * the observable is set.
 */
#pragma once

#include <poco/poco.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct observable_u32 {
    uint32_t value;
    uint32_t target;
} ObservableU32;

/*!
 * @brief Create an observable with an initial value.
 *
 * @param observable Observable to initialise.
 * @param initial_value Initial value of the observable.
 *
 * @return NULL if the observable failed to initialise.
 */
ObservableU32 *observable_create_static(ObservableU32 *observable,
                                        uint32_t initial_value);

/*!
 * @brief Set the value of the observable.
 *
 * The waiting coroutine will be unblocked if it was waiting for this value.
 *
 * @param observable Observable to set.
 * @param value New value to set.
 */
void observable_set_value(ObservableU32 *observable, uint32_t value);

/*!
 * @brief Wait for the observable to become a certain value.
 *
 * @note The value of the observable may not be the target value when this function
 * returns.
 *
 * @param observable Observable to watch.
 * @param target Target to watch for.
 * @param timeout Timeout to wait until.
 *
 * @retval #RES_OK
 * @retval #RES_TIMEOUT The time has elapsed and the target value was not seen.
 */
Result observable_wait_until(ObservableU32 *observable, uint32_t target,
                             PlatformTick timeout);
