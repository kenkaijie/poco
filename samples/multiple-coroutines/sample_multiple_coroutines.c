/*!
 * @file
 * @brief In this example, 1024 coroutines are created, each one
 *        will produce 2 numbers, then terminate.
 *
 * Due to the order defined within the basic scheduler, the output will just be the
 * numbers 0-2047.
 */
#include <poco/coro.h>
#include <poco/scheduler.h>
#include <poco/schedulers/round_robin.h>
#include <stdio.h>

#define CORO_COUNT (1024) // number of coroutines to spawn
#define STACK_SIZE (DEFAULT_STACK_SIZE)

// As we are creating a large number of homogenous coroutines, we just do it as an
// array.

static coro_t coroutines[CORO_COUNT];
static platform_stack_t coroutine_stacks[CORO_COUNT][STACK_SIZE];

// We are using the same entry for all coroutines.
void producer_task(coro_t *coro, void *context) {
    intptr_t starting_number = (intptr_t)context;
    printf("%d\n", starting_number);
    coro_yield(coro);
    printf("%d\n", starting_number + CORO_COUNT);
    coro_yield(coro);
    return;
}

int main() {

    coro_t *tasks[CORO_COUNT] = {0};

    for (size_t i = 0; i < CORO_COUNT; ++i) {
        tasks[i] =
            coro_create_static(&coroutines[i], producer_task, (void *)(uintptr_t)i,
                               &(coroutine_stacks[i][0]), STACK_SIZE);
    }

    round_robin_scheduler_t *scheduler =
        round_robin_scheduler_create(tasks, CORO_COUNT);

    if (scheduler == NULL) {
        printf("Failed to create scheduler\n");
        return -1;
    }

    scheduler_run((scheduler_t *)scheduler);

    return 0;
}
