// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Sample program demonstrating the creation and use of a custom primitive.
 *
 * In this sample, coro1 simply counts the observable up from 0 to 9.
 *
 * Coroutine 2 is set up to wait until the value 4 is reached, after which it will print
 * a special message.
 */
#include "observable_u32.h"
#include <poco/coro.h>
#include <poco/scheduler.h>
#include <poco/schedulers/round_robin.h>
#include <stdio.h>

#define CONSUMER_STOP (-1)
#define STACK_SIZE (DEFAULT_STACK_SIZE)

Coro coro1_coro = {0};
PlatformStackElement coro1_stack[STACK_SIZE] = {0};

Coro coro2_coro = {0};
PlatformStackElement coro2_stack[STACK_SIZE] = {0};

ObservableU32 observable;

void coro1_task(void *context) {
    ObservableU32 *counter = context;
    for (uint32_t cycle = 0; cycle < 10; ++cycle) {
        coro_yield_delay(50);
        printf("Set to %u\n", cycle);
        observable_set_value(counter, cycle);
    }
}

void coro2_task(void *context) {
    ObservableU32 *counter = context;
    observable_wait_until(counter, 4, PLATFORM_TICKS_FOREVER);
    printf("We have counted to 4!\n");
}

int main(void) {

    ObservableU32 *counter = observable_create_static(&observable, 0);

    if (counter == NULL) {
        printf("Failed to create counter\n");
        return -1;
    }

    Coro *tasks[] = {
        coro_create_static(&coro1_coro, coro1_task, counter, coro1_stack, STACK_SIZE),
        coro_create_static(&coro2_coro, coro2_task, counter, coro2_stack, STACK_SIZE),
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
