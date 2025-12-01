#include <poco/poco.h>

/*!
 * @brief Helper to create a cmocka test running inside a coroutine. If you have your
 * own setup and teardown actions, you call the shim functions directly and handle the
 * states yourself.
 */
#define cmocka_coro_unit_test(f)                                                       \
    {#f, test_coro_run, test_coro_setup, test_coro_teardown, f}

// Special cmocka shim functions to perform unit tests within a coroutine.

int test_coro_setup(void **state);
void test_coro_run(void **state);
int test_coro_teardown(void **state);
