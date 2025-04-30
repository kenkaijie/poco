/*!
 * @file
 * @brief Hello world example, performs the classic hello world example using
 * coroutines.
 *
 * Each task sends either Hello, or World!, and alternates between the two tasks.
 *
 * The result should be 5 instances of the string "Hello world" printed to stdout.
 *
 * This example uses the basic scheduler.
 */
#include <poco/coro.h>
#include <poco/scheduler.h>
#include <poco/schedulers/round_robin.h>
#include <stdio.h>

#define STACK_SIZE (1024)

CORO_STATIC_DEFINE(hello, STACK_SIZE);
CORO_STATIC_DEFINE(world, STACK_SIZE);

void hello_task(coro_t *coro, void *context) {
    for (int i = 0; i < 5; ++i) {
        printf("Hello ");
        coro_yield(coro);
    }
    return;
}

void world_task(coro_t *coro, void *context) {
    for (int i = 0; i < 5; ++i) {
        printf("World!\n");
        coro_yield(coro);
    }
    return;
}

int main() {

    coro_t *tasks[2] = {0};

    tasks[0] = coro_create_static(&hello_coro, hello_task, NULL, hello_stack, STACK_SIZE);
    tasks[1] = coro_create_static(&world_coro, world_task, NULL, world_stack, STACK_SIZE);

    round_robin_scheduler_t *scheduler = round_robin_scheduler_create(tasks, 2);

    if (scheduler == NULL) {
        printf("Failed to create scheduler\n");
        return -1;
    }

    scheduler_run((scheduler_t *)scheduler);

    return 0;
}
