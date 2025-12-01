// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Semaphore communication primitive.
 *
 * Supports both binary and counting semaphores.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <poco/platform.h>
#include <poco/result.h>
#include <stddef.h>

typedef struct semaphore {
    size_t volatile slots_remaining;
    size_t slot_count;
} Semaphore;

/*!
 * @brief Create a binary semaphore of a particular size.
 *
 * @note This is equivalent to creating a semaphore with slot_count set to 1.
 *
 * @return the created semaphore, or NULL.
 */
Semaphore *semaphore_create_binary(void);

/*!
 * @brief Initialise a statically allocated binary semaphore.
 *
 * @note This is equivalent to creating a semaphore with slot_count set to 1.
 *
 * @param semaphore Semaphore to initialise.
 *
 * @return the same semaphore as the input.
 */
Semaphore *semaphore_create_binary_static(Semaphore *semaphore);

/*!
 * @brief Create a bounded semaphore of a particular size.
 *
 * @param slot_count Maximum number of allowed concurrent acquisitions.
 *
 * @return the created semaphore, or NULL.
 */
Semaphore *semaphore_create(size_t slot_count);

/*!
 * @brief Initialise a statically allocated semaphore.
 *
 * @param semaphore Semaphore to initialise.
 * @param slot_count Maximum number of allowed concurrent acquisitions.
 *
 * @return the same semaphore as the input.
 */
Semaphore *semaphore_create_static(Semaphore *semaphore, size_t slot_count);

/*!
 * @brief Frees a semaphore created using the dynamic allocation functions.
 *
 * @warning Freeing a semaphore not created by semaphore_create* is causes undefined
 * behaviour.
 *
 * @param semaphore Semaphore to free (binary or counting).
 */
void semaphore_free(Semaphore *semaphore);

/*!
 * @brief Acquire the semaphore, waiting forever.
 *
 * @param semaphore Semaphore to acquire.
 * @param delay_ticks Number of ticks to wait before timing out.
 *
 * @retval #RES_OK If semaphore was acquired
 * @retval #RES_TIMEOUT The maximum timeout duration has been reached.
 */
Result semaphore_acquire(Semaphore *semaphore, PlatformTicks delay_ticks);

/*!
 * @brief Release the semaphore.
 *
 * @param semaphore Semaphore to release.
 *
 * @retval #RES_OK Semaphore has been released.
 * @retval #RES_OVERFLOW Semaphore has already hit the maximum number of releases.
 *                       (A double release has occurred.)
 */
Result semaphore_release(Semaphore *semaphore);

/*!
 * @brief Acquire the semaphore from an ISR, does not block.
 *
 * @note This should be called from an ISR context only.
 *
 * @param semaphore Semaphore to acquire.
 *
 * @retval #RES_OK semaphore has been acquired
 * @retval #RES_TIMEOUT if the semaphore cannot be acquired.
 */
Result semaphore_acquire_from_isr(Semaphore *semaphore);

/*!
 * @brief Release the semaphore from an ISR, does not block.
 *
 * @note This should be called from an ISR context only.
 *
 * @param semaphore Semaphore to acquire.
 *
 * @retval #RES_OK semaphore has been released.
 * @retval #RES_OVERFLOW if the semaphore could not be released.
 * @retval #RES_NOTIFY_FAILED if the scheduler notification has failed.
 */
Result semaphore_release_from_isr(Semaphore *semaphore);

#ifdef __cplusplus
}
#endif
