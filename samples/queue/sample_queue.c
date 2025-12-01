// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Queue example, showcases the use of queues and the scheduler.
 *
 * In this example, one task will produce data every 500 milliseconds, the consumer
 * task will consume whatever values come through.
 *
 * The producer then signals the consumer to end task by sending a sentinel value.
 */

#include <poco/poco.h>
#include <stdio.h>

#define CONSUMER_STOP (-1)

#define STACK_SIZE (DEFAULT_STACK_SIZE)

#define QUEUE_COUNT (10)

void producer_task(void *context) {
    Queue *queue = context;
    for (int i = 0; i < 3; ++i) {
        queue_put(queue, &i, PLATFORM_TICKS_FOREVER);
        printf("Put %d\n", i);
        coro_yield_delay(1000);
    }
    int const sentinel = CONSUMER_STOP;
    queue_put(queue, &sentinel, PLATFORM_TICKS_FOREVER);
    printf("Put %d\n", sentinel);
}

void consumer_task(void *context) {
    Queue *queue = context;
    while (1) {
        int received_value = 0;
        queue_get(queue, &received_value, PLATFORM_TICKS_FOREVER);
        printf("Got: %d\n", received_value);

        if (received_value == CONSUMER_STOP) {
            printf("Done\n");
            break;
        }
        coro_yield();
    }
}

int main(void) {

    Queue *queue_handle = queue_create(QUEUE_COUNT, sizeof(int));

    Coro *producer_handle = coro_create(producer_task, queue_handle, STACK_SIZE);

    if (producer_handle == NULL) {
        /* Memory error */
        return -1;
    }

    Coro *consumer_handle = coro_create(consumer_task, queue_handle, STACK_SIZE);

    if (consumer_handle == NULL) {
        /* Memory error */
        return -1;
    }

    Coro *tasks[] = {producer_handle, consumer_handle};

    Scheduler *scheduler =
        round_robin_scheduler_create(tasks, sizeof(tasks) / sizeof(tasks[0]));

    if (scheduler == NULL) {
        printf("Failed to create scheduler\n");
        return -1;
    }

    scheduler_run(scheduler);

    /* Everything finished, no need to free as the process will terminate. */

    return 0;
}
