// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Unix platform specific operations.
 *
 * Note as per the platform integration, none of the platform functions are guaranteed
 * to actually be functions.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <ucontext.h>

// Platform Context implementation
typedef uint32_t PlatformStackElement;

/** Default stack size needed to run the coroutine, in platform specific elements. */
#define DEFAULT_STACK_SIZE (SIGSTKSZ / sizeof(PlatformStackElement))

/** Minimum stack size needed to run the coroutine, in platform specific elements. */
#define MIN_STACK_SIZE (MINSIGSTKSZ / sizeof(PlatformStackElement))

typedef ucontext_t PlatformContext;

#define platform_get_context(context) getcontext(context)

#define platform_set_context(context) setcontext(context)

#define platform_swap_context(old_context, new_context)                                \
    swapcontext(old_context, new_context)

#define platform_make_context(context, function, coro, user_context)                   \
    makecontext(context, (void (*)(void))function, 2, coro, user_context)

#define platform_destroy_context(context) // no context to destroy

// Platform Timing
typedef int64_t PlatformTicks;

#define PLATFORM_TICKS_FOREVER (INT64_MIN)

static inline PlatformTicks platform_get_monotonic_ticks(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (PlatformTicks)ts.tv_sec * 1000 + (PlatformTicks)ts.tv_nsec / 1000000;
}

#define platform_get_ticks_per_ms() (1)

#define platform_enter_critical_section()
#define platform_exit_critical_section()

#ifdef __cplusplus
}
#endif
