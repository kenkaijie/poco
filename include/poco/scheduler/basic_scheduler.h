/*!
 * @file
 * @brief A very basic scheduler, just loops throught round robin. Used primarily for
 *        demos.
 * 
 * This scheduler does not handle the "advanced" corotuine signals, which will
 * result in tighter spin locks.
 * 
 * For sleeping, it uses time.h to perform timing.
 * 
 * Limitations:
 * 
 * - Only supports static coroutines.
 * - Uses malloc.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <poco/coro.h>
#include <poco/error.h>
#include <stddef.h>

typedef struct {
    coro_t ** tasks;
    size_t task_count;
    size_t finished_tasks;
    size_t current_task_index; /**< index of the currently running task */
} basic_scheduler_t;

/*!
 * @brief Create a basic scheduler.
 *
 * @param coro_list List of coroutines to schedule.
 * @param num_coros Number of coroutines in the list.
 * 
 * @return Pointer to the scheduler, or NULL on error.
 */
basic_scheduler_t * basic_scheduler_create(coro_t ** coro_list, size_t num_coros);

/*!
 * @brief Run the scheduler.
 *
 * @note This will block until all coroutines are finished.
 * 
 * @param scheduler Scheduler to run.
 */
void basic_scheduler_run(basic_scheduler_t * scheduler);

#ifdef __cplusplus
}
#endif
