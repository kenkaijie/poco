/*!
 * @file
 * @brief Tests event implementation.
 */

#include "cmocka_coro_helper.h"
#include <poco/event.h>
#include <poco/poco.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// cmocka has to be included after its dependencies
#include <cmocka.h>

// Just a quick switch to enable/disable malloc
static bool disable_malloc = false;

extern void *__real_malloc(size_t);

void *__wrap_malloc(size_t size) {
    return (disable_malloc) ? NULL : __real_malloc(size);
}

/*!
 * @brief Test malloc failure is propagated.
 */
static void test_malloc_failure(void **state) {
    disable_malloc = true;
    event_t *event = event_create(0);
    disable_malloc = false;
    assert_null(event);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_coro_unit_test(test_malloc_failure),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
