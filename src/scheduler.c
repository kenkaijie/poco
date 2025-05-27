#include <poco/scheduler.h>
#include <poco/context.h>

void scheduler_run(scheduler_t * scheduler)
{
    context_set_scheduler(scheduler);
    return scheduler->run(scheduler);
}