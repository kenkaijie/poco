/*!
 * @file
 * @brief Copy of the README sample, just to ensure it works.
 * 
 * Will print "hello world!" forever.
 */

#include <poco/poco.h>
#include <stdio.h>

// Prints "hello world!" every 1 second.
static void my_coroutine(void * context) {
    for (;;) {
        printf("hello world!\n");
        coro_yield_delay(1000);
    }
}

int main(int argc, char *argv[]) {
    // ...

    coro_t * tasks[1] = {
        coro_create(my_coroutine, NULL, DEFAULT_STACK_SIZE)
        /* add other tasks */
    };

    // ...

    scheduler_t *scheduler = round_robin_scheduler_create(tasks, 1);

    /* Will never return */
    scheduler_run(scheduler);
}
