/*! 
 * @file
 * @brief Mutex for resource ownership on a single coroutine.
 * 
 * This does not have an ISR API, as mutexes are not no be used within an ISR context
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
    coro_t * owner;
} mutex_t;

/*!
 * @brief Initialise a statically allocated mutex.
 *
 * @param mutex Mutex to initialise.
 * 
 * @returns Pointer to the mutex.
 */
mutex_t * mutex_create_static(mutex_t * mutex);

/*!
 * @brief Dynamically create and initialise a mutex.
 *
 * @returns Pointer to a mutex, or NULL if it cannot be created.
 */
mutex_t * mutex_create(void);

/*!
 * @brief Free a previously created dynamic mutex.
 *
 * @warning Freeing a mutex not created by mutex_create is causes undefined behaviour.
 * 
 * @param mutex Mutex to free.
 */
void mutex_free(mutex_t * mutex);

/*!
 * @brief Acquires a resource exclusively for this coroutine.
 *
 * @note Repeated calls from the same coroutine are allowed.
 * 
 * @param coro Current running coroutine.
 * @param mutex Mutex to acquire.
 * @param timeout Maximum time to wait before giving up.
 * 
 * @retval #RES_OK Mutex was acquired.
 * @retval #RES_TIMEOUT Timeout occurred.
 */
result_t mutex_acquire(coro_t * coro, mutex_t * mutex, platform_ticks_t timeout);

/*!
 * @brief Acquires a resource exclusively for this coroutine without waiting.
 *
 * @note Repeated calls from the same coroutine are allowed.
 * 
 * @param coro Current running coroutine.
 * @param mutex Mutex to acquire.
 * @param timeout Maximum time to wait before giving up.
 * 
 * @retval #RES_OK Mutex was acquired.
 * @retval #RES_MUTEX_OCCUPIED Mutex is currently occupied by another coroutine.
 */
result_t mutex_acquire_no_wait(coro_t * coro, mutex_t * mutex);

/*!
 * @brief Releases the mutex.
 *
 * @note This call is idempotent.
 * 
 * @retval #RES_OK Mutex was released.
 * @retval #RES_MUTEX_NOT_OWNER
 */
result_t mutex_release(coro_t * coro, mutex_t * mutex);
