/*!
 * @file
 * @brief Standard error return type.
 *
 * This defines an error space for both the poco library and the wider system.
 *
 * All poco errors required a 65535 sized error space. By default, we reserve
 * the error space 0x00000000 to 0x0000FFFF.
 *
 * Within the poco library error space, it is split between 0xXXYY, where XX
 * denotes the module and YY denotes the error code.
 *
 * This file defines the numbers assigned to each module. The specific error codes
 * will be defined within the module itself (with the exception of the general
 * error space).
 */

#pragma once

#include <stdint.h>

typedef uint32_t error_t;

/*!
 * @brief Macro to define an error code within the poco error space (0xXXYY).
 *
 * @param module Module number (XX).
 * @param code Error code (YY).
 */
#define MODULE_ERROR(module, code) ((error_t)(((module) << 16) | (code)))

enum module_space {
    MODULE_GENERAL = 0, /**< General space for common error codes. */
    MODULE_CORE = 1,
    MODULE_QUEUE = 2,
    MODULE_EVENT = 3,
};

#define RET_OK MODULE_ERROR(MODULE_GENERAL, 0) /**< No error. */
#define RET_NOT_OWNER                                                                  \
    MODULE_ERROR(MODULE_GENERAL,                                                       \
                 1) /**< Not the owner of the mutex, or thread owned object. */
#define RET_NO_MEM MODULE_ERROR(MODULE_GENERAL, 2) /**< No memory available. */