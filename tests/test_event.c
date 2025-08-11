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

/*!
 * @brief Test we can free a created event;
 */
static void test_event_create_and_free(void **state) {
    event_t *event = event_create(0);
    event_free(event);
}

/*!
 * @brief An event that is waiting on a specific mask should ignore bit changes outside
 *        of the mask.
 */
static void test_event_wait_on_specific_bit(void **state) {
    event_t *event = event_create(0);

    event_set(event, 0x1);

    flags_t result = event_get(event, 0x2, 0x02, false, 200);

    assert_int_equal(result, 0);
}

static void _bit_set_for_test_event_wait_on_all_bit(void *context) {
    event_t *event = (event_t *)context;

    for (size_t idx = 0; idx < 4; ++idx) {
        event_set(event, 0x01 << idx);
        coro_yield();
    }
}

/*!
 * @brief An event that is waiting on multiple bits with the wait for all set to true
 *        should keep on blocking until all bits are set.
 */
static void test_event_wait_on_all_bit(void **state) {
    event_t *event = event_create(0);

    round_robin_scheduler_add_coro((round_robin_scheduler_t *)context_get_scheduler(),
                                   coro_create(_bit_set_for_test_event_wait_on_all_bit,
                                               event, DEFAULT_STACK_SIZE));

    flags_t result = event_get(event, 0xF, 0x0F, true, PLATFORM_TICKS_FOREVER);

    assert_int_equal(result, 0xF);
}

/*!
 * @brief An event can be set from the ISR.
 *
 * Note we can use ISR calls in a non ISR context as long as we are certain no interupts
 * can preempt us. For testing, this assumption is usually ok.
 */
static void test_setting_from_isr(void **state) {
    event_t *event = event_create(0);
    result_t isr_result = RES_OK;

    isr_result = event_set_from_isr(event, 0x80);

    flags_t result = event_get(event, 0x80, 0x080, true, PLATFORM_TICKS_FOREVER);

    assert_int_equal(result, 0x80);
}

/*!
 * @brief Setting more than the allowable queued values causes a failure.
 */
static void test_event_setting_from_isr_notify_failure(void **state) {
    event_t *event = event_create(0);
    result_t isr_result = RES_OK;

    for (size_t count = 0; count < SCHEDULER_MAX_EXTERNAL_EVENT_COUNT; ++count) {
        isr_result = event_set_from_isr(event, 0x80);
        assert_int_equal(isr_result, RES_OK);
    }

    // last set should result in an error
    isr_result = event_set_from_isr(event, 0x80);
    assert_int_equal(isr_result, RES_NOTIFY_FAILED);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_coro_unit_test(test_event_create_and_free),
        cmocka_coro_unit_test(test_event_wait_on_specific_bit),
        cmocka_coro_unit_test(test_event_wait_on_all_bit),
        cmocka_coro_unit_test(test_setting_from_isr),
        cmocka_coro_unit_test(test_event_setting_from_isr_notify_failure),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
