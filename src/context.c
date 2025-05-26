/*!
 * @file
 * @brief Context implementation for single threaded scheduler support.
 */

#include <poco/context.h>
#include <threads.h>

static scheduler_t *assigned_scheduler = NULL;

scheduler_t *context_get_scheduler(void) { return assigned_scheduler; }

coro_t *context_get_coro(void) {
    return scheduler_get_current_coroutine(assigned_scheduler);
}

void context_set_scheduler(scheduler_t *scheduler) { assigned_scheduler = scheduler; }
