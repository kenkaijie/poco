/*!
 * @file
 * @brief Queue example, showcases the use of queues and the scheduler.
 * 
 * In this example, one task will produce data every 500 milliseconds, the consumer
 * task will consume whatever values come through.
 * 
 * The producer then signals the consumer to end task by sending a sentinel value.
 */
#include <poco/coro.h>
#include <poco/queue.h>
#include <poco/scheduler/basic_scheduler.h>
#include <stdio.h>

#define CONSUMER_STOP (0xFFFFFFFF)

CORO_STATIC_DEFINE(producer, 1024);
CORO_STATIC_DEFINE(consumer, 1024);

QUEUE_STATIC_DEFINE(numbers, 10, int);

void producer_task(coro_t *coro, void* context)
{
    queue_t * queue = (queue_t *)context;
    for (int i=0; i < 3; ++i)
    {
        queue_put(coro, queue, (void*)&i);
        printf("Put %d\n", i);
        coro_yield_delay(coro, 1000);
    }
    int sentinel = CONSUMER_STOP;
    queue_put(coro, queue, (void *)&sentinel);
    printf("Put %d\n", sentinel);
    return;
}

void consumer_task(coro_t *coro, void* context)
{
    queue_t * queue = (queue_t *)context;
    while(1)
    {
        int received_value = 0;
        queue_get(coro, queue, (void *) &received_value);
        printf("Got: %d\n", received_value);

        if (received_value == CONSUMER_STOP)
        {
            printf("Done\n");
            break;
        }
        coro_yield(coro);
    }
    return;
}

int main() {

    coro_t* tasks[2] = {0};

    queue_t * queue = queue_create_static(&numbers_queue, 10, sizeof(int), numbers_queue_buffer);

    tasks[0] = coro_create_static(&producer_coro, producer_task, (void *)queue, producer_stack, sizeof(producer_stack));
    tasks[1] = coro_create_static(&consumer_coro, consumer_task, (void *)queue, consumer_stack, sizeof(consumer_stack));

    basic_scheduler_t * scheduler = basic_scheduler_create(tasks, 2);

    if (scheduler == NULL)
    {
        printf("Failed to create scheduler\n");
        return -1;
    }

    basic_scheduler_run(scheduler);

    return 0;
}
