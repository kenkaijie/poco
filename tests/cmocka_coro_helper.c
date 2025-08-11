#include <poco/poco.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// cmocka has to be included after its dependencies
#include <cmocka.h>

typedef struct test_coro_context {
    coro_t *tasks[16];
    scheduler_t *scheduler;
} test_coro_context_t;

/*!
 * Special cmocka shim to perform unit tests within a coroutine.
 */
int test_coro_setup(void **state) {
    CMUnitTestFunction test_function = *state;

    test_coro_context_t *context =
        (test_coro_context_t *)malloc(sizeof(test_coro_context_t));

    memset(context, 0, sizeof(test_coro_context_t));

    context->tasks[0] =
        coro_create((coro_function_t)test_function, NULL, DEFAULT_STACK_SIZE);
    context->scheduler = round_robin_scheduler_create(context->tasks, 16);
    *state = context;
    return 0;
}

void test_coro_run(void **state) {
    test_coro_context_t *context = (test_coro_context_t *)*state;
    scheduler_run(context->scheduler);
}

int test_coro_teardown(void **state) {
    test_coro_context_t *context = (test_coro_context_t *)*state;

    /* perform all teardowns */
    for (size_t idx = 0; idx < 16; ++idx) {
        coro_free(context->tasks[idx]);
    }

    round_round_robin_scheduler_free((round_robin_scheduler_t *)context->scheduler);

    free(context);

    return 0;
}
