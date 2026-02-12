// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
#include <poco/poco.h>
#include <stdio.h>
#include <string.h>

// cmocka requires these dependencies
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
// cmocka also needs to be the last included
#include <cmocka.h>

#define MAX_TASK_COUNT (16)

typedef struct test_coro_context {
    Coro *tasks[MAX_TASK_COUNT];
    Scheduler *scheduler;
} TestCoroContext;

/*!
 * Special cmocka shim to perform unit tests within a coroutine.
 */
int test_coro_setup(void **state) {
    CMUnitTestFunction const test_function = *state;

    TestCoroContext *context = malloc(sizeof(TestCoroContext));

    memset(context, 0, sizeof(TestCoroContext));

    context->tasks[0] =
        coro_create((CoroEntrypoint)test_function, NULL, DEFAULT_STACK_SIZE);
    context->scheduler = round_robin_scheduler_create(context->tasks, MAX_TASK_COUNT);
    *state = context;
    return 0;
}

void test_coro_run(void **state) {
    TestCoroContext const *context = *state;
    scheduler_run(context->scheduler);
}

int test_coro_teardown(void **state) {
    TestCoroContext *context = *state;

    /* perform all teardowns */
    for (size_t idx = 0; idx < MAX_TASK_COUNT; ++idx) {
        coro_free(context->tasks[idx]);
    }

    round_round_robin_scheduler_free((RoundRobinScheduler *)context->scheduler);

    free(context);

    return 0;
}
