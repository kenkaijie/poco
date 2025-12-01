// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Example of using streams to send byte data between coroutines.
 *
 * Here a producer coro sends a few messages.
 *
 * The consumer will read the expected number fo bytes, then print it to the
 * console.
 *
 * It's just lorem ipsum.
 */

#include <poco/poco.h>
#include <stdio.h>
#include <string.h>

#define STACK_SIZE (DEFAULT_STACK_SIZE)

static char const *messages[13] = {
    "Dear sir/madam,\n",
    "\n",
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec viverra lacus\n",
    "ligula, vel fringilla ipsum egestas vel. Mauris cursus nisl massa, eget\n",
    "volutpat ante fermentum ut. Interdum et malesuada fames ac ante ipsum primis in\n",
    "faucibus. Fusce porttitor sed tortor non posuere. Phasellus facilisis massa\n",
    "consequat congue blandit. In ligula dui, feugiat sed tristique quis, vestibulum\n",
    "eu quam. Nunc ullamcorper luctus sapien, a elementum massa viverra vel. Etiam\n",
    "eget tellus mi. Nam accumsan turpis ut nisi pellentesque molestie. Donec\n",
    "euismod placerat dignissim. Nunc tortor mi, varius nec posuere pharetra,\n",
    "malesuada quis elit. Aenean iaculis ornare dolor, nec consectetur leo\n",
    "pellentesque vel. Nulla dapibus, erat eget accumsan volutpat, justo ante\n",
    "condimentum dolor, vitae ornare massa nulla in ligula. Sed eu hendrerit libero.\n",
};

static void producer_task(void *context) {
    Stream *stream = (Stream *)context;

    for (size_t idx = 0; idx < (sizeof(messages) / sizeof(messages[0])); ++idx) {
        char const *message = messages[idx];
        size_t message_len = strlen(message);
        stream_send(stream, (uint8_t const *)message, &message_len,
                    PLATFORM_TICKS_FOREVER);
    }
    stream_flush(stream, PLATFORM_TICKS_FOREVER);
}

static void consumer_task(void *context) {
    Stream *stream = (Stream *)context;

    uint8_t recv_buffer[128] = {0};
    // keep last byte for a null terminating character
    size_t buffer_len = sizeof(recv_buffer) - 1;

    while (buffer_len > 0) {
        buffer_len = sizeof(recv_buffer) - 1;
        stream_receive_up_to(stream, recv_buffer, &buffer_len,
                             1000 * platform_get_ticks_per_ms());
        recv_buffer[buffer_len] = '\0';
        printf("%s", recv_buffer);
    }
}

int main(void) {

    Stream *stream = stream_create(1024);

    if (stream == NULL) {
        printf("Failed to create stream.");
        return -1;
    }

    Coro *tasks[] = {
        coro_create(producer_task, (void *)stream, STACK_SIZE),
        coro_create(consumer_task, (void *)stream, STACK_SIZE),
    };

    for (size_t idx = 0; idx < (sizeof(tasks) / sizeof(tasks[0])); ++idx) {
        if (tasks[idx] == NULL)
            /* Memory error */
            return -1;
    }

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
