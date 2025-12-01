/*!
 * @file
 * @brief Tests queue implementation.
 */

#include "cmocka_coro_helper.h"
#include <poco/poco.h>
#include <poco/queue.h>
#include <string.h>

// cmocka requires these dependencies
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
// cmocka also needs to be the last included
#include <cmocka.h>

/** Test item for queue testing. */
typedef struct dummy_item {
    uint32_t a;
    uint8_t b;
    uint32_t c;
} dummy_item_t;

/*!
 * @brief Tests a full queue returns the correct with the no_wait API.
 */
static void test_queue_push_no_wait_to_full(void **context) {
    Result result = RES_OK;
    Queue *queue = queue_create(1, sizeof(dummy_item_t));
    dummy_item_t const item = {
        .a = 1,
        .b = 2,
        .c = 12,
    };

    // first time should pass
    result = queue_put_no_wait(queue, &item);
    assert_int_equal(RES_OK, result);

    // second should fail
    result = queue_put_no_wait(queue, &item);
    assert_int_equal(RES_QUEUE_FULL, result);
}

/*!
 * @brief Tests a full queue push with the plain API.
 */
static void test_queue_push_to_full(void **context) {
    Result result = RES_OK;
    Queue *queue = queue_create(1, sizeof(dummy_item_t));
    dummy_item_t const item = {
        .a = 1,
        .b = 2,
        .c = 12,
    };

    // first time should pass
    result = queue_put(queue, &item, 0);
    assert_int_equal(RES_OK, result);

    // second should fail, but since we are using the coro itnerface, it will return
    // timeout (instead of FULL).
    result = queue_put(queue, &item, 0);
    assert_int_equal(RES_TIMEOUT, result);
}

/*!
 * @brief Tests we can get and fetch items from a queue.
 */
static void test_queue_put_and_get(void **context) {
    Result result = RES_OK;
    int const expected_item = 55;
    int actual_item = 0;

    Queue *queue = queue_create(1, sizeof(int));

    result = queue_put(queue, &expected_item, PLATFORM_TICKS_FOREVER);
    assert_int_equal(RES_OK, result);

    result = queue_get(queue, &actual_item, PLATFORM_TICKS_FOREVER);
    assert_int_equal(RES_OK, result);

    assert_int_equal(expected_item, actual_item);
}

void put_coro_for_test_queue_put_and_get(void *context) {
    Queue *queue = (Queue *)context;
    int const expected_item = 55;
    queue_put(queue, &expected_item, PLATFORM_TICKS_FOREVER);
};

/*!
 * @brief Tests we can get and fetch items from a queue, in reverse order.
 */
static void test_queue_put_and_get_reverse_order(void **context) {
    Result result = RES_OK;
    int const expected_item = 55;
    int actual_item = 0;

    Queue *queue = queue_create(1, sizeof(int));

    Coro *put_coro = coro_create(put_coro_for_test_queue_put_and_get, (void *)queue,
                                 DEFAULT_STACK_SIZE);

    round_robin_scheduler_add_coro((RoundRobinScheduler *)context_get_scheduler(),
                                   put_coro);

    coro_join(put_coro);

    result = queue_get(queue, &actual_item, PLATFORM_TICKS_FOREVER);
    assert_int_equal(RES_OK, result);

    assert_int_equal(expected_item, actual_item);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_coro_unit_test(test_queue_push_no_wait_to_full),
        cmocka_coro_unit_test(test_queue_push_to_full),
        cmocka_coro_unit_test(test_queue_put_and_get),
        cmocka_coro_unit_test(test_queue_put_and_get_reverse_order),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
