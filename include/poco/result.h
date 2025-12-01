// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Common result codes used within the poco library.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint32_t Result;

/*!
 * @brief Macro to define an error code within the poco error space (0xXXYY).
 *
 * @param group Group this error code belongs to.
 * @param code Error code (YY).
 */
#define RES_CODE(group, code) ((Result)(((group) << 16) | (code)))

/* These categories should not change once allocated to avoid breaking changes with the
 * Result API.
 */
enum result_category {
    RES_GROUP_GENERAL = 0, /**< General space for common error codes. */
    RES_GROUP_CORE = 1,
    RES_GROUP_QUEUE = 2,
    RES_GROUP_EVENT = 3,
    RES_GROUP_MUTEX = 4,
    RES_GROUP_STREAM = 5,
    RES_GROUP_SEMAPHORE = 7,
};

/*!
 * @brief General purpose error codes.
 */
enum res_codes_general {
    /*! No error. */
    RES_OK = RES_CODE(RES_GROUP_GENERAL, 0),

    /*! No memory available. */
    RES_NO_MEM = RES_CODE(RES_GROUP_GENERAL, 1),

    /*! Cannot perform operation in the current state. */
    RES_INVALID_STATE = RES_CODE(RES_GROUP_GENERAL, 2),

    /*! Provided value is outside the expected range. */
    RES_INVALID_VALUE = RES_CODE(RES_GROUP_GENERAL, 3),

    /*! Operation that would cause an overflow has occurred. */
    RES_OVERFLOW = RES_CODE(RES_GROUP_GENERAL, 4),

    /*! Operation exceeded maximum allowable time. */
    RES_TIMEOUT = RES_CODE(RES_GROUP_GENERAL, 5),

    /*!
     * A scheduler notification failed.
     *
     * @warning This is a critical failure and indicates the number of events the
     * scheduler has been configured with is not sufficient.
     */
    RES_NOTIFY_FAILED = RES_CODE(RES_GROUP_GENERAL, 6),
};

#ifdef __cplusplus
}
#endif