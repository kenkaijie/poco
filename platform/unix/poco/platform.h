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

#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <ucontext.h>

// Platform Context implementation
typedef uint32_t platform_stack_t;

typedef ucontext_t platform_context_t;

#define platform_get_context(context) getcontext(context)

#define platform_set_context(context) setcontext(context)

#define platform_swap_context(old_context, new_context)                                \
    swapcontext(old_context, new_context)

#define platform_make_context(context, function, coro, user_context)                   \
    makecontext(context, (void (*)(void))function, 2, coro, user_context)

// Platform Timing
typedef int64_t platform_ticks_t;

__attribute__((always_inline)) static inline platform_ticks_t
platform_get_monotonic_ticks(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (platform_ticks_t)ts.tv_sec * 1000 + (platform_ticks_t)ts.tv_nsec / 1000000;
}

#define platform_get_ticks_per_ms() (1)

#define platform_enter_critical_section()
#define platform_exit_critical_section()

#ifdef __cplusplus
}
#endif
