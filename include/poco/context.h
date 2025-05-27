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
scheduler_t *context_get_scheduler(void);

coro_t *context_get_coro(void);

/*!
 * @brief Sets the scheduler for this context.
 *
 * @param scheduler Scheduler to set.
 */
void context_set_scheduler(scheduler_t *scheduler);

#ifdef __cplusplus
}
#endif
