#
# Tests samples.
#
# SPDX-FileCopyrightText: Copyright contributors to the poco project.
# SPDX-License-Identifier: MIT

# Function wrapper for tests that compare their outputs with a golden sample.
#  
# Inputs:
#   TEST_NAME: name of the test, will generated both the test and the output test
#   TEST_TARGET: name of the target to run, will be used to generate the test
function(add_golden_test TEST_NAME TEST_TARGET)
    add_test(
        NAME ${TEST_NAME}
        COMMAND 
            ${CMAKE_CURRENT_LIST_DIR}/test_output.sh
            $<TARGET_FILE:${TEST_TARGET}>
            $<TARGET_FILE_DIR:${TEST_TARGET}>/${TEST_NAME}.actual
            ${CMAKE_CURRENT_LIST_DIR}/${TEST_NAME}.expected
    )
endfunction()

add_golden_test(test_sample_aggregate_wait sample_aggregate_wait)
add_golden_test(test_sample_coro_joining sample_coro_joining)
add_golden_test(test_sample_hello_world sample_hello_world)
add_golden_test(test_sample_multiple_coroutines sample_multiple_coroutines)
add_golden_test(test_sample_mutex sample_mutex)
add_golden_test(test_sample_queue_overflow sample_queue_overflow)
add_golden_test(test_sample_queue sample_queue)
add_golden_test(test_sample_semaphore sample_semaphore)
add_golden_test(test_sample_simple_event sample_simple_event)
add_golden_test(test_sample_stream sample_stream)
