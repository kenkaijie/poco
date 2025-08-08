/*!
 * @file
 * @brief Example showcasing the recipe of a single coroutine waiting on multiple
 *        communication primitives.
 *
 * This example shows how use of events can allow a coroutine to perform a
 * "wait for any" style of blocking for multiple communication primitives at a
 * time.abort
 *
 * Here, we have 2 queues to be awaited.
 *
 * SPDX-FileCopyrightText: Copyright contributors to the poco project.
 * SPDX-License-Identifier: MIT
 */

#include "consumer.h"
#include <poco/coro.h>
#include <poco/event.h>
#include <poco/queue.h>
#include <poco/scheduler.h>
#include <poco/schedulers/round_robin.h>
#include <stdio.h>

#define STACK_SIZE (DEFAULT_STACK_SIZE)

CORO_STATIC_DEFINE(producer_1, STACK_SIZE);
CORO_STATIC_DEFINE(producer_2, STACK_SIZE);

consumer_t consumer;

void producer_1_task(void *context) {

    consumer_t *consumer = (consumer_t *)context;

    coro_yield_delay(100);
    message_t message = {
        .a = 12,
        .b = 11,
    };
    printf("Send message, a=%d, b=%d.\n", message.a, message.b);
    consumer_send_message(consumer, &message);
    coro_yield_delay(100);
    message.a = 15;
    printf("Send message, a=%d, b=%d.\n", message.a, message.b);
    consumer_send_message(consumer, &message);
    message.a = 16;
    printf("Send message, a=%d, b=%d.\n", message.a, message.b);
    consumer_send_message(consumer, &message);
    return;
}

void producer_2_task(void *context) {
    consumer_t *consumer = (consumer_t *)context;

    coro_yield_delay(100);
    command_t command = {
        .a = 1,
        .b = 2,
        .c = 3,
    };
    printf("Send command, a=%d, b=%d, c=%d.\n", command.a, command.b, command.c);
    consumer_send_command(consumer, &command);
    coro_yield_delay(100);
    command.a = 4;
    printf("Send command, a=%d, b=%d, c=%d.\n", command.a, command.b, command.c);
    consumer_send_command(consumer, &command);
    command.a = 5;
    printf("Send command, a=%d, b=%d, c=%d.\n", command.a, command.b, command.c);
    consumer_send_command(consumer, &command);
    command.a = -1;
    printf("Send command, a=%d, b=%d, c=%d.\n", command.a, command.b, command.c);
    consumer_send_command(consumer, &command);
    return;
}

int main(void) {

    coro_t *tasks[3] = {0};

    result_t init_success = consumer_init(&consumer);

    if (init_success != RES_OK) {
        printf("Failed to initialise consumer.");
        return -1;
    }

    tasks[0] = coro_create_static(&producer_1_coro, producer_1_task, &consumer,
                                  producer_1_stack, STACK_SIZE);
    tasks[1] = coro_create_static(&producer_2_coro, producer_2_task, &consumer,
                                  producer_2_stack, STACK_SIZE);
    tasks[2] = &consumer.coro;

    scheduler_t *scheduler =
        round_robin_scheduler_create(tasks, sizeof(tasks) / sizeof(tasks[0]));

    if (scheduler == NULL) {
        printf("Failed to create scheduler\n");
        return -1;
    }

    scheduler_run(scheduler);

    return 0;
}
