/*!
 * @file
 * @brief Consumer example implementation.
 *
 * SPDX-FileCopyrightText: Copyright contributors to the poco project.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <poco/coro.h>
#include <poco/event.h>
#include <poco/platform.h>
#include <poco/queue.h>

#define CONSUMER_SIG_MESSAGE (1 << 0) /** Queue 1 has an item. */
#define CONSUMER_SIG_COMMAND (1 << 1) /** Queue 2 has an item. */
#define CONSUMER_SIG_ALL (CONSUMER_SIG_MESSAGE | CONSUMER_SIG_COMMAND)

typedef struct message {
    int a;
    int b;
} message_t;

typedef struct command {
    int a;
    int b;
    int c;
} command_t;

typedef struct consumer {
    platform_stack_t stack[DEFAULT_STACK_SIZE];
    coro_t coro;
    event_t event;
    message_t message_queue_buffer[16];
    queue_t message_queue;
    command_t command_queue_buffer[16];
    queue_t command_queue;
} consumer_t;

result_t consumer_init(consumer_t *consumer);

/*!
 * @brief Sends a message via the consumer.
 */
result_t consumer_send_message(consumer_t *consumer, message_t const *message);

/*!
 * @brief Sends a command via the consumer.
 */
result_t consumer_send_command(consumer_t *consumer, command_t const *message);
