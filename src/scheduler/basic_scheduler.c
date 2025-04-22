#include <poco/scheduler/basic_scheduler.h>
#include <poco/intercoro.h>
#include <poco/platform.h>
#include <string.h>


size_t increment_task_index(basic_scheduler_t * scheduler)
{
    return (scheduler->current_task_index + 1) % scheduler->task_count;
}

static size_t get_finished_task_count(basic_scheduler_t * scheduler)
{
    size_t finished_tasks = 0;
    for (size_t i=0; i < scheduler->task_count; ++i)
    {
        if (scheduler->tasks[i]->coro_state == CORO_STATE_FINISHED)
        {
            finished_tasks++;
        }
    }
    return finished_tasks;
}

static coro_t * get_next_ready_task(basic_scheduler_t * scheduler)
{
    for (size_t offset = 0; offset < scheduler->task_count; ++offset)
    {
        size_t task_index = (scheduler->current_task_index + offset) % scheduler->task_count;
        if (scheduler->tasks[task_index]->coro_state == CORO_STATE_READY)
        {
            scheduler->current_task_index = increment_task_index(scheduler);
            return scheduler->tasks[task_index];
        }
    }
    return NULL;
}

/*!
 * @brief Check and update the event sink with the new event.
 *
 * @return true if the owner of the event can be unblocked.
 */
static bool update_event_sink(coro_event_sink_t * sink, coro_event_source_t const * event)
{
    bool unblock_task = false;

    switch (event->type)
    {
        case CORO_EVTSRC_ELAPSED:
            if (sink->type == CORO_EVTSINK_DELAY)
            {
                sink->params.ticks_remaining -= event->params.elasped_ticks;
                unblock_task = (sink->params.ticks_remaining <= 0);
            }
            break;
        case CORO_EVTSRC_QUEUE_GET:
            if (sink->type == CORO_EVTSINK_QUEUE_NOT_FULL)
            {
                unblock_task = (sink->params.queue == event->params.queue);
            }
            break;
        case CORO_EVTSRC_QUEUE_PUT:
            if (sink->type == CORO_EVTSINK_QUEUE_NOT_EMPTY)
            {
                unblock_task = (sink->params.queue == event->params.queue);
            }
            break;
        default:
            unblock_task = false;
    }
    return unblock_task;
}

/*!
 * @brief Update waiting tasks with the event.
 */
static void update_waiting_tasks(basic_scheduler_t * scheduler, coro_event_source_t const * event)
{
    for (size_t i=0; i < scheduler->task_count; ++i)
    {
        coro_t * task = scheduler->tasks[i];
        if (task->coro_state == CORO_STATE_BLOCKED)
        {
            for (size_t j=0; j < 2; ++j)
            {
                bool unblock_task = update_event_sink(&task->event_sinks[j], event);
                if (unblock_task)
                {
                    task->coro_state = CORO_STATE_READY;
                    coro_reset_sinks(task); // Reset the sinks.
                    break;
                }
            }
        }
    }
}



basic_scheduler_t * basic_scheduler_create(coro_t ** coro_list, size_t num_coros)
{
    basic_scheduler_t * scheduler = malloc(sizeof(basic_scheduler_t));
    if (scheduler == NULL)
    {
        /* No more memory. */
        return NULL;
    }

    scheduler->tasks = (coro_t **)malloc(num_coros * sizeof(coro_t*));
    scheduler->task_count = num_coros;
    scheduler->finished_tasks = 0;
    scheduler->current_task_index = 0;

    if (scheduler->tasks == NULL)
    {
        /* No more memory. */
        return NULL;
    }

    for (size_t i=0; i < num_coros; ++i)
    {
        scheduler->tasks[i] = coro_list[i];
    }

    return scheduler;
}

void basic_scheduler_run(basic_scheduler_t * scheduler)
{
    scheduler->finished_tasks = get_finished_task_count(scheduler);

    platform_ticks_t previous_ticks = platform_get_monotonic_ticks();

    while(scheduler->finished_tasks < scheduler->task_count)
    {
        coro_t * next_coro = get_next_ready_task(scheduler);
        // If there are no tasks to run, we can just wait for the next event.
        if (next_coro != NULL)
        {
            next_coro->coro_state = CORO_STATE_RUNNING;
            coro_signal_type_t signal = coro_resume(next_coro);

            coro_event_source_t * coroutine_event = NULL;

            switch (signal)
            {
                case CORO_SIG_NOTIFY:
                    next_coro->coro_state = CORO_STATE_READY;
                    coroutine_event = &next_coro->event_source;
                    break;
                case CORO_SIG_DONE:
                    next_coro->coro_state = CORO_STATE_FINISHED;
                    scheduler->finished_tasks++;
                    break;
                case CORO_SIG_WAIT:
                    next_coro->coro_state = CORO_STATE_BLOCKED;
                    break;
                case CORO_SIG_NOTIFY_AND_WAIT:
                    next_coro->coro_state = CORO_STATE_BLOCKED;
                    coroutine_event = &next_coro->event_source;
                    break;
            }
            if (coroutine_event != NULL)
            {
                update_waiting_tasks(scheduler, coroutine_event);
            }
        }

        platform_ticks_t current_ticks = platform_get_monotonic_ticks();

        // Process a time signal, which is synthesized within the scheduler.
        if (current_ticks == previous_ticks)
        {
            // No time has passed, so we can just skip this.
            continue;
        }
        coro_event_source_t time_event = {
            .type = CORO_EVTSINK_DELAY,
            .params.elasped_ticks = (current_ticks - previous_ticks),
        };

        previous_ticks = current_ticks;

        update_waiting_tasks(scheduler, &time_event);
        
    }
}
