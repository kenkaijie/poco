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

#define STACK_SIZE (1024)

void coroutine_a(coro_t *coro, void *context) {
    mutex_t *mutex = (mutex_t *)context;
    
    mutex_acquire(coro, mutex, PLATFORM_TICKS_FOREVER);

    for (int i = 0; i < 4; ++i) {
        printf("A%d\n", i);
    }

    mutex_release(coro, mutex);

}

void coroutine_b(coro_t *coro, void *context) {
    mutex_t *mutex = (mutex_t *)context;
    
    mutex_acquire(coro, mutex, PLATFORM_TICKS_FOREVER);

    for (int i = 0; i < 4; ++i) {
        printf("B%d\n", i);
    }

    mutex_release(coro, mutex);
}

int main() {

    mutex_t *mutex = mutex_create();

    if (mutex == NULL) {
        /* Memory error */
        return -1;
    }

    coro_t *coro_a =
        coro_create(coroutine_a, (void *)mutex, STACK_SIZE);

    if (coro_a == NULL) {
        /* Memory error */
        return -1;
    }

    coro_t *coro_b =
        coro_create(coroutine_b, (void *)mutex, STACK_SIZE);

    if (coro_b == NULL) {
        /* Memory error */
        return -1;
    }

    coro_t *tasks[] = {coro_a, coro_b};

    round_robin_scheduler_t *scheduler =
        round_robin_scheduler_create(tasks, sizeof(tasks) / sizeof(tasks[0]));

    if (scheduler == NULL) {
        printf("Failed to create scheduler\n");
        return -1;
    }

    scheduler_run((scheduler_t *)scheduler);

    /* Everything finished, no need to free as the process will terminate. */
    return 0;
}
