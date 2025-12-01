// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Consumer example implementation.
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
} Message;

typedef struct command {
    int a;
    int b;
    int c;
} Command;

typedef struct consumer {
    PlatformStackElement stack[DEFAULT_STACK_SIZE];
    Coro coro;
    Event event;
    Message message_queue_buffer[16];
    Queue message_queue;
    Command command_queue_buffer[16];
    Queue command_queue;
} Consumer;

Result consumer_init(Consumer *consumer);

/*!
 * @brief Sends a message via the consumer.
 */
Result consumer_send_message(Consumer *consumer, Message const *message);

/*!
 * @brief Sends a command via the consumer.
 */
Result consumer_send_command(Consumer *consumer, Command const *message);
