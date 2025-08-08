/*!
 * @file
 * @brief Example of coroutine synchronisation using semaphores.
 *
 * Here a resource is guarded by a semaphore of size 2. So 2 of the 3 accessors will
 * be able to perform their actions.
 *
 * The third coroutine waits for the first accessor to complete, then it will begin.
 *
 * The expected order is thus:
 *
 * - 1 Acquired
 * - 2 Acquired
 * - 1 Released
 * - 3 Acquired
 * - 2 Released
 * - 3 Released
 *
 * SPDX-FileCopyrightText: Copyright contributors to the poco project.
 * SPDX-License-Identifier: MIT
 */

#include <poco/poco.h>
#include <stdio.h>

#define STACK_SIZE (DEFAULT_STACK_SIZE)

void accessor_1(void *context) {
    semaphore_t *resource = (semaphore_t *)context;

    semaphore_acquire(resource, PLATFORM_TICKS_FOREVER);

    printf("1 Acquired\n");
    coro_yield_delay(500);
    printf("1 Released\n");

    semaphore_release(resource);
}

void accessor_2(void *context) {
    semaphore_t *resource = (semaphore_t *)context;

    semaphore_acquire(resource, PLATFORM_TICKS_FOREVER);

    printf("2 Acquired\n");
    coro_yield_delay(1000);
    printf("2 Released\n");

    semaphore_release(resource);
}

void accessor_3(void *context) {
    semaphore_t *resource = (semaphore_t *)context;

    semaphore_acquire(resource, PLATFORM_TICKS_FOREVER);

    printf("3 Acquired\n");
    coro_yield_delay(500);
    printf("3 Released\n");

    semaphore_release(resource);
}

int main(void) {

    semaphore_t *resource = semaphore_create(2);

    if (resource == NULL) {
        /* Memory error */
        return -1;
    }

    coro_t *tasks[] = {
        coro_create(accessor_1, (void *)resource, STACK_SIZE),
        coro_create(accessor_2, (void *)resource, STACK_SIZE),
        coro_create(accessor_3, (void *)resource, STACK_SIZE),
    };

    for (size_t idx = 0; idx < (sizeof(tasks) / sizeof(tasks[0])); ++idx) {
        if (tasks[idx] == NULL)
            /* Memory error */
            return -1;
    }

    scheduler_t *scheduler =
        round_robin_scheduler_create(tasks, sizeof(tasks) / sizeof(tasks[0]));

    if (scheduler == NULL) {
        printf("Failed to create scheduler\n");
        return -1;
    }

    scheduler_run(scheduler);

    /* Everything finished, no need to free as the process will terminate. */
    return 0;
}
