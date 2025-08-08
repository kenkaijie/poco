/*!
 * @file
 * @brief Sample consumer object for testing aggregated waits.
 *
 * SPDX-FileCopyrightText: Copyright contributors to the poco project.
 * SPDX-License-Identifier: MIT
 */

#include "consumer.h"
#include <stdio.h>

static void consumer_loop(consumer_t *consumer) {
    while (1) {
        printf("Waiting on message or command.\n");
        flags_t events = event_get(&consumer->event, CONSUMER_SIG_ALL, CONSUMER_SIG_ALL,
                                   false, PLATFORM_TICKS_FOREVER);
        if (events & CONSUMER_SIG_COMMAND) {
            command_t command = {0};
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
            message_t message = {0};
            queue_get_no_wait(&consumer->message_queue, (void *)&message);
            printf("Received a message, a=%d, b=%d.\n", message.a, message.b);
            if (queue_item_count(&consumer->message_queue) > 0) {
                event_set(&consumer->event, CONSUMER_SIG_MESSAGE);
            }
        }
    }
}

result_t consumer_init(consumer_t *consumer) {
    coro_create_static(&consumer->coro, (coro_function_t)consumer_loop, consumer,
                       consumer->stack, DEFAULT_STACK_SIZE);

    queue_create_static(&consumer->message_queue,
                        sizeof(consumer->message_queue_buffer) /
                            (sizeof(*consumer->message_queue_buffer)),
                        sizeof(message_t), (uint8_t *)consumer->message_queue_buffer);
    queue_create_static(&consumer->command_queue,
                        sizeof(consumer->command_queue_buffer) /
                            (sizeof(*consumer->command_queue_buffer)),
                        sizeof(command_t), (uint8_t *)consumer->command_queue_buffer);

    event_create_static(&consumer->event, 0);

    return RES_OK;
}

result_t consumer_send_message(consumer_t *consumer, message_t const *message) {
    result_t put_success = queue_put_no_wait(&consumer->message_queue, (void *)message);
    if (put_success == RES_OK) {
        event_set(&consumer->event, CONSUMER_SIG_MESSAGE);
    }
    return put_success;
}

result_t consumer_send_command(consumer_t *consumer, command_t const *message) {
    result_t put_success = queue_put_no_wait(&consumer->command_queue, (void *)message);
    if (put_success == RES_OK) {
        event_set(&consumer->event, CONSUMER_SIG_COMMAND);
    }
    return put_success;
}
