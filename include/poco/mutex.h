// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Mutex for resource ownership on a single coroutine.
 *
 * This does not have an ISR API, as mutexes are not to be used within an ISR context
 * (they are purely a coroutine primitive).
 */

#pragma once

#include <poco/coro.h>
#include <poco/platform.h>
#include <poco/result.h>

enum res_codes_mutex {
    /* Mutex cannot be freed, as this coroutine is not the owner. */
    RES_MUTEX_NOT_OWNER = RES_CODE(RES_GROUP_MUTEX, 0),

    /* Mutex cannot be acquired as it is occupied by another coroutine. */
    RES_MUTEX_OCCUPIED = RES_CODE(RES_GROUP_MUTEX, 1),
};

typedef struct mutex {
    Coro *owner;
} Mutex;

/*!
 * @brief Initialise a statically allocated mutex.
 *
 * @param mutex Mutex to initialise.
 *
 * @returns Pointer to the mutex.
 */
Mutex *mutex_create_static(Mutex *mutex);

/*!
 * @brief Dynamically create and initialise a mutex.
 *
 * @returns Pointer to a mutex, or NULL if it cannot be created.
 */
Mutex *mutex_create(void);

/*!
 * @brief Free a previously created dynamic mutex.
 *
 * @warning Freeing a mutex not created by mutex_create is causes undefined behaviour.
 *
 * @param mutex Mutex to free.
 */
void mutex_free(Mutex *mutex);

/*!
 * @brief Acquires a resource exclusively for this coroutine.
 *
 * @note Repeated calls from the same coroutine are allowed.
 *
 * @param mutex Mutex to acquire.
 * @param timeout Maximum time to wait before giving up.
 *
 * @retval #RES_OK Mutex was acquired.
 * @retval #RES_TIMEOUT Timeout occurred.
 */
Result mutex_acquire(Mutex *mutex, PlatformTicks timeout);

/*!
 * @brief Acquires a resource exclusively for this coroutine without waiting.
 *
 * @note Repeated calls from the same coroutine are allowed.
 *
 * @param mutex Mutex to acquire.
 *
 * @retval #RES_OK Mutex was acquired.
 * @retval #RES_MUTEX_OCCUPIED Mutex is currently occupied by another coroutine.
 */
Result mutex_acquire_no_wait(Mutex *mutex);

/*!
 * @brief Releases the mutex.
 *
 * @note This call is idempotent.
 *
 * @param mutex Mutex to release.
 *
 * @retval #RES_OK Mutex was released.
 * @retval #RES_MUTEX_NOT_OWNER
 */
Result mutex_release(Mutex *mutex);
