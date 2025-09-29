// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief User context (ucontext) inspired API implementation for Cortex M4.
 *
 * Uses some Zephyr related primitives for time keeping and critical sections.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <zephyr/irq.h>
#include <zephyr/kernel.h>

/** M4 requires stacks to be 8 byte aligned. */
typedef uint64_t platform_stack_t;

#define DEFAULT_STACK_SIZE (256 / sizeof(platform_stack_t))
#define MIN_STACK_SIZE (DEFAULT_STACK_SIZE)

typedef struct machine_context {
    uint32_t volatile r0;     // 0x00
    uint32_t volatile r1;     // 0x04
    uint32_t volatile r2;     // 0x08
    uint32_t volatile r3;     // 0x0C
    uint32_t volatile r4;     // 0x10
    uint32_t volatile r5;     // 0x14
    uint32_t volatile r6;     // 0x18
    uint32_t volatile r7;     // 0x1C
    uint32_t volatile r8;     // 0x20
    uint32_t volatile r9;     // 0x24
    uint32_t volatile r10;    // 0x28
    uint32_t volatile r11;    // 0x2C
    uint32_t volatile r12;    // 0x30
    uint32_t volatile r13_sp; // 0x34 (R13 Stack Pointer)
    uint32_t volatile r14_lr; // 0x38 (R14 Link Register)
    uint32_t volatile r15_pc; // 0x3C (R15 Program Counter)
} mcontext_t;

typedef struct stack_descriptor {
    /**< Pointer to a stack, note that stack must be double word aligned. */
    void *ss_sp;
    size_t ss_size; /**< Stack size, in bytes */

} stack_descriptor_t;

typedef struct platform_context platform_context_t;

struct platform_context {
    struct platform_context *uc_link;
    stack_descriptor_t uc_stack;       // Stack information
    mcontext_t uc_mcontext; // Machine context
};

/*!
 * @brief Initialises the context to the current user context of the calling thread.
 *
 * This includes machine context and the current execution stack.
 *
 * @param[in] context Context structure to save state in.
 * @return 0 on success, -1 otherwise.
 */
extern int platform_get_context(platform_context_t *context);

/*!
 * @brief Restore the use context defined in context.
 *
 * A successful call does not return.
 *
 * @param[in] context Context to restore.
 * @return will not return on success, -1 on failure.
 */
extern int platform_set_context(const platform_context_t *context);

extern int platform_swap_context(platform_context_t *current_context,
                                 const platform_context_t *new_context);

/*!
 * @brief Create a new context and set the entry point to the function func.
 *
 * @note This implementation uses a hardcoded 3 arguments.
 */
void platform_make_context(platform_context_t *context, void (*func)(void *, void *),
                           void *context1, void *context2);

#define platform_destroy_context(context) // no context to destroy

typedef int64_t platform_ticks_t;

#define PLATFORM_TICKS_FOREVER (INT64_MIN)

static inline platform_ticks_t platform_get_monotonic_ticks(void) {
    // This case is safe as our platform (NRF) casts it from a uint64 to int64 anyway.
    return (uint64_t)k_uptime_ticks();
}

// FIXME: This is not accurate, as sys tick is not directly convertible to
//        milliseocnds, we will have a small drift over time with this.
#define platform_get_ticks_per_ms() (CONFIG_SYS_CLOCK_TICKS_PER_SEC / 1000)

#define platform_enter_critical_section() int key = irq_lock()
#define platform_exit_critical_section() irq_unlock(key)

#ifdef __cplusplus
}
#endif
