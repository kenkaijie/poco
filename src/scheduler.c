#include <poco/context.h>
#include <poco/scheduler.h>

void scheduler_run(scheduler_t *scheduler) {
    context_set_scheduler(scheduler);
    return scheduler->run(scheduler);
}