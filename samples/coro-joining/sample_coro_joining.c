/*!
 * @file
 * @brief An example showing the join mechanics.
 *
 * In this sample, coro1 will run to completetion before coro2, as coro2 will join
 * coro1 before printing its values.
 *
 * SPDX-FileCopyrightText: Copyright contributors to the poco project.
 * SPDX-License-Identifier: MIT
 */

#include <poco/coro.h>
#include <poco/event.h>
#include <poco/scheduler.h>
#include <poco/schedulers/round_robin.h>
#include <stdio.h>

#define CONSUMER_STOP (-1)
#define STACK_SIZE (DEFAULT_STACK_SIZE)

coro_t coro1_coro = {0};
platform_stack_t coro1_stack[STACK_SIZE] = {0};

coro_t coro2_coro = {0};
platform_stack_t coro2_stack[STACK_SIZE] = {0};

void coro1_task(void *context) {
    (void)context;
    for (int cycle = 0; cycle < 5; ++cycle) {
        printf("A=%d\n", cycle);
        coro_yield();
    }
}

void coro2_task(void *context) {
    coro_t *coro1 = (coro_t *)context;

    coro_join(coro1);

    for (int cycle = 0; cycle < 5; ++cycle) {
        printf("B=%d\n", cycle);
        coro_yield();
    }
}

int main(void) {

    coro_t *tasks[2] = {0};

    tasks[0] =
        coro_create_static(&coro1_coro, coro1_task, NULL, coro1_stack, STACK_SIZE);
    tasks[1] = coro_create_static(&coro2_coro, coro2_task, (void *)tasks[0],
                                  coro2_stack, STACK_SIZE);

    scheduler_t *scheduler =
        round_robin_scheduler_create(tasks, sizeof(tasks) / sizeof(tasks[0]));

    if (scheduler == NULL) {
        printf("Failed to create scheduler\n");
        return -1;
    }

    scheduler_run(scheduler);

    return 0;
}
