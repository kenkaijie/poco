// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Hello world example, performs the classic hello world example using
 * coroutines.
 *
 * Each task sends either Hello, or World!, and alternates between the two tasks.
 *
 * The result should be 5 instances of the string "Hello world" printed to stdout.
 *
 * This example uses the basic round robin scheduler.
 */

#include <poco/poco.h>
#include <stdio.h>

#define STACK_SIZE (DEFAULT_STACK_SIZE)

void hello_task(void *context) {
    (void)context;
    for (int i = 0; i < 5; ++i) {
        printf("Hello ");
        coro_yield();
    }
}

void world_task(void *context) {
    (void)context;
    for (int i = 0; i < 5; ++i) {
        printf("World!\n");
        coro_yield();
    }
}

int main() {

    Coro *tasks[] = {
        coro_create(hello_task, NULL, STACK_SIZE),
        coro_create(world_task, NULL, STACK_SIZE),
    };

    Scheduler *scheduler =
        round_robin_scheduler_create(tasks, sizeof(tasks) / sizeof(tasks[0]));

    if (scheduler == NULL) {
        printf("Failed to create scheduler\n");
        return -1;
    }

    scheduler_run(scheduler);

    return 0;
}
