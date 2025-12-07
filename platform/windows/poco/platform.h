// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Windows platform specific operations.
 *
 * We use the Fiber API to emulate the same coroutine behaviour.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <windows.h>

// Platform Context implementation, in windows, as the stack is managed by windows
// internally, this shouldn't matter, as we don't use it.
typedef uint32_t PlatformStackElement;

/** Default stack size needed to run the coroutine, in platform specific elements. */
#define DEFAULT_STACK_SIZE (3) /* In windows, stack is always dynamic */

/** Minimum stack size needed to run the coroutine, in platform specific elements. */
#define MIN_STACK_SIZE (3) /* In windows, stack is always dynamic */

typedef struct stack_descriptor {
    /**< Pointer to a stack, note that stack must be double word aligned. */
    void *ss_sp;
    size_t ss_size; /**< Stack size, in bytes */

} StackDescriptor;

typedef struct platform_context PlatformContext;

struct platform_context {
    /* In the Windows implementation, we don't use any of the fields here, just the
     * fiber pointer. */
    struct platform_context *uc_link;
    StackDescriptor uc_stack;
    LPVOID fiber;
};

int platform_get_context(PlatformContext *context);

int platform_set_context(PlatformContext *context);

int platform_swap_context(PlatformContext *old_context, PlatformContext *new_context);

int platform_make_context(PlatformContext *context, void (*entrypoint)(void *, void *),
                          void *coro, void *user_context);

int platform_destroy_context(PlatformContext *context);

// Platform Timing
typedef int64_t PlatformTick;

#define PLATFORM_TICKS_FOREVER (INT64_MAX)

static inline PlatformTick platform_get_monotonic_ticks(void) {
    LARGE_INTEGER freq, counter;
    // Apparently post Windows XP, these 2 calls can never fail.
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    // Convert to milliseconds
    return (counter.QuadPart * 1000) / freq.QuadPart;
}

#define platform_get_ticks_per_ms() (1)

#define platform_enter_critical_section()
#define platform_exit_critical_section()

#ifdef __cplusplus
}
#endif
