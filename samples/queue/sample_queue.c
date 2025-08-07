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
#include <poco/scheduler.h>
#include <poco/schedulers/round_robin.h>
#include <stdio.h>

#define CONSUMER_STOP (-1)

#define STACK_SIZE (DEFAULT_STACK_SIZE)

#define QUEUE_COUNT (10)

void producer_task(void *context) {
    queue_t *queue = (queue_t *)context;
    for (int i = 0; i < 3; ++i) {
        queue_put(queue, (void *)&i, PLATFORM_TICKS_FOREVER);
        printf("Put %d\n", i);
        coro_yield_delay(1000);
    }
    int sentinel = CONSUMER_STOP;
    queue_put(queue, (void *)&sentinel, PLATFORM_TICKS_FOREVER);
    printf("Put %d\n", sentinel);
    return;
}

void consumer_task(void *context) {
    queue_t *queue = (queue_t *)context;
    while (1) {
        int received_value = 0;
        queue_get(queue, (void *)&received_value, PLATFORM_TICKS_FOREVER);
        printf("Got: %d\n", received_value);

        if (received_value == CONSUMER_STOP) {
            printf("Done\n");
            break;
        }
        coro_yield();
    }
    return;
}

int main(void) {

    queue_t *queue_handle = queue_create(QUEUE_COUNT, sizeof(int));

    coro_t *producer_handle =
        coro_create(producer_task, (void *)queue_handle, STACK_SIZE);

    if (producer_handle == NULL) {
        /* Memory error */
        return -1;
    }

    coro_t *consumer_handle =
        coro_create(consumer_task, (void *)queue_handle, STACK_SIZE);

    if (consumer_handle == NULL) {
        /* Memory error */
        return -1;
    }

    coro_t *tasks[] = {producer_handle, consumer_handle};

    scheduler_t *scheduler =
        round_robin_scheduler_create(tasks, sizeof(tasks) / sizeof(tasks[0]));

    if (scheduler == NULL) {
        printf("Failed to create scheduler\n");
        return -1;
    }

    scheduler_run(scheduler);

    /* Everything finished, no need to free as the process will terminate. */

    return 0;
}
