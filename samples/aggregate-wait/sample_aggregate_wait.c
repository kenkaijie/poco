// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Example showcasing the recipe of a single coroutine waiting on multiple
 *        communication primitives.
 *
 * This example shows how use of events can allow a coroutine to perform a
 * "wait for any" style of blocking for multiple communication primitives at a
 * time.
 *
 * Here, we have 2 producers, 1 sending commands, and another sending messages.
 *
 * The consumer task can wait for either queues as the send command and send message
 * functions set an event.
 */

#include "consumer.h"
#include <poco/coro.h>
#include <poco/event.h>
#include <poco/queue.h>
#include <poco/scheduler.h>
#include <poco/schedulers/round_robin.h>
#include <stdio.h>

#define STACK_SIZE (DEFAULT_STACK_SIZE)

coro_t producer_1_coro = {0};
platform_stack_t producer_1_stack[STACK_SIZE] = {0};

coro_t producer_2_coro = {0};
platform_stack_t producer_2_stack[STACK_SIZE] = {0};

consumer_t consumer;

void producer_1_task(void *context) {

    consumer_t *consumer = (consumer_t *)context;
    message_t message = {
        .a = 12,
        .b = 11,
    };
    printf("Send message, a=%d, b=%d.\n", message.a, message.b);
    consumer_send_message(consumer, &message);
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
    command_t command = {
        .a = 1,
        .b = 2,
        .c = 3,
    };
    printf("Send command, a=%d, b=%d, c=%d.\n", command.a, command.b, command.c);
    consumer_send_command(consumer, &command);
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
