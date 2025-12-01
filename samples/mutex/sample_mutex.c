// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Example of coroutine synchronisation using mutexes.
 *
 * In this sample, even if the coroutines are using the roun robin scheduler. Due to
 * coroutine A and B sharing a mutex, the output will be all A's, then all B's.
 */

#include <poco/coro.h>
#include <poco/mutex.h>
#include <poco/scheduler.h>
#include <poco/schedulers/round_robin.h>
#include <stdio.h>

#define STACK_SIZE (DEFAULT_STACK_SIZE)

void coroutine_a(void *context) {
    Mutex *mutex = context;

    mutex_acquire(mutex, PLATFORM_TICKS_FOREVER);

    for (int i = 0; i < 4; ++i) {
        printf("A%d\n", i);
    }

    mutex_release(mutex);
}

void coroutine_b(void *context) {
    Mutex *mutex = context;

    mutex_acquire(mutex, PLATFORM_TICKS_FOREVER);

    for (int i = 0; i < 4; ++i) {
        printf("B%d\n", i);
    }

    mutex_release(mutex);
}

int main(void) {

    Mutex *mutex = mutex_create();

    if (mutex == NULL) {
        /* Memory error */
        return -1;
    }

    Coro *coro_a = coro_create(coroutine_a, mutex, STACK_SIZE);

    if (coro_a == NULL) {
        /* Memory error */
        return -1;
    }

    Coro *coro_b = coro_create(coroutine_b, mutex, STACK_SIZE);

    if (coro_b == NULL) {
        /* Memory error */
        return -1;
    }

    Coro *tasks[] = {coro_a, coro_b};

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
