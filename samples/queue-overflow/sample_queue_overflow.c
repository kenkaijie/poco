/*!
 * @file
 * @brief Queue example with overflows.
 *
 * In this example, the producer wants to put 20 items in the queue, but the consumer
 * is set to be slow in taking values. The queue also only has 5 slots, so the producer
 * will block until the consumer takes some values.
 *
 */
#include <poco/coro.h>
#include <poco/queue.h>
#include <poco/scheduler.h>
#include <poco/schedulers/round_robin.h>
#include <stdio.h>

#define CONSUMER_STOP (0xFFFFFFFF)

CORO_STATIC_DEFINE(producer, 1024);
CORO_STATIC_DEFINE(consumer, 1024);

QUEUE_STATIC_DEFINE(numbers, 5, int);

void producer_task(coro_t *coro, void *context) {
    queue_t *queue = (queue_t *)context;
    for (int i = 0; i < 20; ++i) {
        queue_put(coro, queue, (void *)&i);
        printf("Put %d\n", i);
        coro_yield(coro);
    }
    int sentinel = CONSUMER_STOP;
    queue_put(coro, queue, (void *)&sentinel);
    printf("Put %d\n", sentinel);
    return;
}

void consumer_task(coro_t *coro, void *context) {
    queue_t *queue = (queue_t *)context;
    while (1) {
        int received_value = 0;
        queue_get(coro, queue, (void *)&received_value);
        printf("Got: %d\n", received_value);

        if (received_value == CONSUMER_STOP) {
            printf("Done\n");
            break;
        }
        coro_yield_delay(coro, 100);
    }
    return;
}

int main() {

    coro_t *tasks[2] = {0};

    queue_t *queue =
        queue_create_static(&numbers_queue, 5, sizeof(int), numbers_queue_buffer);

    tasks[0] = coro_create_static(&producer_coro, producer_task, (void *)queue,
                                  producer_stack, sizeof(producer_stack));
    tasks[1] = coro_create_static(&consumer_coro, consumer_task, (void *)queue,
                                  consumer_stack, sizeof(consumer_stack));

    round_robin_scheduler_t *scheduler = round_robin_scheduler_create(tasks, 2);

    if (scheduler == NULL) {
        printf("Failed to create scheduler\n");
        return -1;
    }

    scheduler_run((scheduler_t *)scheduler);

    return 0;
}
