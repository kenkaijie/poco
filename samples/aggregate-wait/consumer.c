// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Sample consumer object for testing aggregated waits.
 */

#include "consumer.h"
#include <stdio.h>

static void consumer_loop(Consumer *consumer) {
    while (1) {
        printf("Waiting on message or command.\n");
        Flags events = event_get(&consumer->event, CONSUMER_SIG_ALL, CONSUMER_SIG_ALL,
                                 false, PLATFORM_TICKS_FOREVER);
        if (events & CONSUMER_SIG_COMMAND) {
            Command command = {0};
            queue_get_no_wait(&consumer->command_queue, (void *)&command);
            printf("Received a command, a=%d, b=%d, c=%d.\n", command.a, command.b,
                   command.c);

            if (command.a == -1) {
                break;
            }

            if (queue_item_count(&consumer->command_queue) > 0) {
                event_set(&consumer->event, CONSUMER_SIG_COMMAND);
            }
        }

        if (events & CONSUMER_SIG_MESSAGE) {
            Message message = {0};
            queue_get_no_wait(&consumer->message_queue, (void *)&message);
            printf("Received a message, a=%d, b=%d.\n", message.a, message.b);
            if (queue_item_count(&consumer->message_queue) > 0) {
                event_set(&consumer->event, CONSUMER_SIG_MESSAGE);
            }
        }
    }
}

Result consumer_init(Consumer *consumer) {
    coro_create_static(&consumer->coro, (CoroFunction)consumer_loop, consumer,
                       consumer->stack, DEFAULT_STACK_SIZE);

    queue_create_static(&consumer->message_queue,
                        sizeof(consumer->message_queue_buffer) /
                            (sizeof(*consumer->message_queue_buffer)),
                        sizeof(Message), (uint8_t *)consumer->message_queue_buffer);
    queue_create_static(&consumer->command_queue,
                        sizeof(consumer->command_queue_buffer) /
                            (sizeof(*consumer->command_queue_buffer)),
                        sizeof(Command), (uint8_t *)consumer->command_queue_buffer);

    event_create_static(&consumer->event, 0);

    return RES_OK;
}

Result consumer_send_message(Consumer *consumer, Message const *message) {
    Result put_success = queue_put_no_wait(&consumer->message_queue, (void *)message);
    if (put_success == RES_OK) {
        event_set(&consumer->event, CONSUMER_SIG_MESSAGE);
    }
    return put_success;
}

Result consumer_send_command(Consumer *consumer, Command const *command) {
    Result put_success = queue_put_no_wait(&consumer->command_queue, (void *)command);
    if (put_success == RES_OK) {
        event_set(&consumer->event, CONSUMER_SIG_COMMAND);
    }
    return put_success;
}
