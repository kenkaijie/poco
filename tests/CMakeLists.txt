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
    if (WIN32)
        add_test(
            NAME ${TEST_NAME}_exec
            COMMAND ${CMAKE_CURRENT_LIST_DIR}/test_output.bat $<TARGET_FILE:${TEST_TARGET}> ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}.actual
        )
    else()
        add_test(
            NAME ${TEST_NAME}_exec
            COMMAND ${CMAKE_CURRENT_LIST_DIR}/test_output.sh $<TARGET_FILE:${TEST_TARGET}> ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}.actual
        )
    endif()
    add_test(
        NAME ${TEST_NAME}
        COMMAND 
            ${CMAKE_COMMAND} -E compare_files --ignore-eol
            ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}.actual
            ${CMAKE_CURRENT_LIST_DIR}/${TEST_NAME}.expected
    )
    set_tests_properties(${TEST_NAME} PROPERTIES DEPENDS ${TEST_NAME}_exec)
endfunction()

# Adds a cmocka test to CTest.
#
# Inputs:
#   TEST_NAME name of the test.
#   SRCS sources to include as part of the test runner
function(add_cmocka_test TEST_NAME SRCS)
    add_executable(${TEST_NAME}_runner ${SRCS} cmocka_coro_helper.c)
    target_link_libraries(${TEST_NAME}_runner PRIVATE poco::poco cmocka)
    add_test(NAME ${TEST_NAME}_runner COMMAND ${TEST_NAME}_runner)
    set_target_properties(${TEST_NAME}_runner PROPERTIES C_STANDARD 99)

    # Windows cannot find the built cmocka DLL to link with. We add the cmocka
    # dll directory to the test environment.
    if (WIN32)
        set_property(
            TEST ${TEST_NAME}_runner
            PROPERTY
                ENVIRONMENT_MODIFICATION  "PATH=path_list_append:$<TARGET_FILE_DIR:cmocka>")
    endif()
endfunction()

if (POCO_BUILD_SAMPLES)
    add_golden_test(test_sample_aggregate_wait sample_aggregate_wait)
    add_golden_test(test_sample_coro_joining sample_coro_joining)
    add_golden_test(test_sample_custom_primitive sample_custom_primitive)
    add_golden_test(test_sample_hello_world sample_hello_world)
    add_golden_test(test_sample_multiple_coroutines sample_multiple_coroutines)
    add_golden_test(test_sample_mutex sample_mutex)
    add_golden_test(test_sample_queue_overflow sample_queue_overflow)
    add_golden_test(test_sample_queue sample_queue)
    add_golden_test(test_sample_semaphore sample_semaphore)
    add_golden_test(test_sample_simple_event sample_simple_event)
    add_golden_test(test_sample_stream sample_stream)
endif()

add_cmocka_test(test_event test_event.c)
add_cmocka_test(test_queue test_queue.c)
