/*!
 * @file
 * @brief Semaphore communication primitive.
 *
 * Supports both binary and counting semaphores.
 *
 * SPDX-FileCopyrightText: Copyright contributors to the poco project.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <poco/coro.h>
#include <poco/event.h>
#include <poco/platform.h>
#include <poco/result.h>
#include <stddef.h>

typedef struct semaphore {
    size_t slots_remaining;
    size_t slot_count;
} semaphore_t;

/*!
 * @brief Create a binary semaphore of a particular size.
 *
 * @note This is equivalent to creating a semaphore with slot_count set to 1.
 *
 * @return the created semaphore, or NULL.
 */
semaphore_t *semaphore_create_binary(void);

/*!
 * @brief Initialise a statically allocated binary semaphore.
 *
 * @note This is equivalent to creating a semaphore with slot_count set to 1.
 *
 * @param semaphore Semaphore to initialise.
 *
 * @return the same semaphore as the input.
 */
semaphore_t *semaphore_create_binary_static(semaphore_t *semaphore);

/*!
 * @brief Create a bounded semaphore of a particular size.
 *
 * @param slot_count Maximum number of allowed concurrent acquisitions.
 *
 * @return the created semaphore, or NULL.
 */
semaphore_t *semaphore_create(size_t slot_count);

/*!
 * @brief Initialise a statically allocated semaphore.
 *
 * @param semaphore Semaphore to initialise.
 * @param slot_count Maximum number of allowed concurrent acquisitions.
 *
 * @return the same semaphore as the input.
 */
semaphore_t *semaphore_create_static(semaphore_t *semaphore, size_t slot_count);

/*!
 * @brief Frees a semaphore created using the dyanamic allocation functions.
 *
 * @warning Freeing a sempahore not created by semaphore_create* is causes undefined
 * behaviour.
 *
 * @param semaphore Semaphore to free (binary or counting).
 */
void semaphore_free(semaphore_t *semaphore);

/*!
 * @brief Acquire the semaphore, waiting forever.
 *
 * @param coro Calling coroutine.
 * @param semaphore Semaphore to acquire.
 * @param delay_ticks Number of ticks to wait before timing out.
 *
 * @retval #RES_OK If semaphore was acquired
 * @retval #RES_TIMEOUT The maximum timeout duration has been reached.
 */
result_t semaphore_acquire(semaphore_t *semaphore, platform_ticks_t delay_ticks);

/*!
 * @brief Release the semaphore.
 *
 * @param coro Calling coroutine.
 * @param semaphore Semaphore to release.
 *
 * @retval #RES_OK Semaphore has been released.
 * @retval #RES_OVERFLOW Semaphore has already hit the maximum number of releases.
 *                       (A double release has occurred.)
 */
result_t semaphore_release(semaphore_t *semaphore);

/*!
 * @brief Acquire the semaphore from an ISR, does not block.
 *
 * @note This should be called from an ISR context only.
 *
 * @param semaphore Semaphore to acquire.
 *
 * @retval #RES_OK semaphore has been acquired
 * @retval
 */
result_t semaphore_acquire_from_isr(semaphore_t *semaphore);

/*!
 * @brief Release the semaphore from an ISR, does not block.
 *
 * @note This should be called from an ISR context only.
 *
 * @param semaphore Semaphore to acquire.
 *
 * @retval #RES_OK semaphore has been released
 */
result_t semaphore_release_from_isr(semaphore_t *semaphore);

#ifdef __cplusplus
}
#endif
