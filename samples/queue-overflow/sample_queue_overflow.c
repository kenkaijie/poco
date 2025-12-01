// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Queue example with overflows.
 *
 * In this example, the producer wants to put 20 items in the queue, but the consumer
 * is set to be slow in taking values. The queue also only has 5 slots, so the producer
 * will block until the consumer takes some values.
 */

#include <poco/coro.h>
#include <poco/queue.h>
#include <poco/scheduler.h>
#include <poco/schedulers/round_robin.h>
#include <stdio.h>

#define CONSUMER_STOP (-1)
#define STACK_SIZE (DEFAULT_STACK_SIZE)

Coro producer_coro = {0};
PlatformStackElement producer_stack[STACK_SIZE] = {0};

Coro consumer_coro = {0};
PlatformStackElement consumer_stack[STACK_SIZE] = {0};

Queue numbers_queue = {0};
uint8_t numbers_queue_buffer[5 * sizeof(int)] = {0};

void producer_task(void *context) {
    Queue *queue = context;
    for (int i = 0; i < 20; ++i) {
        queue_put(queue, &i, PLATFORM_TICKS_FOREVER);
        printf("Put %d\n", i);
        coro_yield();
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
        coro_yield_delay(100);
    }
}

int main(void) {

    Coro *tasks[2] = {0};

    Queue *queue =
        queue_create_static(&numbers_queue, 5, sizeof(int), numbers_queue_buffer);

    tasks[0] = coro_create_static(&producer_coro, producer_task, queue, producer_stack,
                                  STACK_SIZE);
    tasks[1] = coro_create_static(&consumer_coro, consumer_task, queue, consumer_stack,
                                  STACK_SIZE);

    Scheduler *scheduler =
        round_robin_scheduler_create(tasks, sizeof(tasks) / sizeof(tasks[0]));

    if (scheduler == NULL) {
        printf("Failed to create scheduler\n");
        return -1;
    }

    scheduler_run(scheduler);

    return 0;
}
