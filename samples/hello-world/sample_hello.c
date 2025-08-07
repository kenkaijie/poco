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

#define STACK_SIZE (DEFAULT_STACK_SIZE)

void hello_task(void *context) {
    (void)context;
    for (int i = 0; i < 5; ++i) {
        printf("Hello ");
        coro_yield();
    }
    return;
}

void world_task(void *context) {
    (void)context;
    for (int i = 0; i < 5; ++i) {
        printf("World!\n");
        coro_yield();
    }
    return;
}

int main() {

    coro_t *tasks[] = {
        coro_create(hello_task, NULL, STACK_SIZE),
        coro_create(world_task, NULL, STACK_SIZE),
    };

    scheduler_t *scheduler =
        round_robin_scheduler_create(tasks, sizeof(tasks) / sizeof(tasks[0]));

    if (scheduler == NULL) {
        printf("Failed to create scheduler\n");
        return -1;
    }

    scheduler_run(scheduler);

    return 0;
}
