/*!
 * @file
 * @brief User context (ucontext) inspired API implementation for cortex m4.
 *
 * Has many shortcuts.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef struct {
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

typedef struct {
    /**< Pointer to a stack, note that stack must be double word aligned. */
    void *ss_sp;
    size_t ss_size; /**< Stack size, in bytes */

} stack_t;

typedef struct ucontext ucontext_t;

typedef struct ucontext {
    struct ucontext *uc_link;
    stack_t uc_stack;       // Stack information
    mcontext_t uc_mcontext; // Machine context
} ucontext_t;

/*!
 * @brief Initialises the context to the current user context of the calling thread.
 *
 * This includes machine context and the current execution stack.
 *
 * @param[in] context Context structure to save state in.
 * @return 0 on success, -1 otherwise.
 */
int platform_get_context(ucontext_t *context);

/*!
 * @brief Restore the use context defined in context.
 *
 * A successful call does not return.
 *
 * @param[in] context Context to restore.
 * @return will not return on success, -1 on failure.
 */
int platform_set_context(const ucontext_t *context);

int platform_swap_context(ucontext_t *current_context, const ucontext_t *new_context);

/*!
 * @brief Create a new context and set the entry point to the function func.
 *
 * @note This implementation uses a hardcoded 3 arguments.
 */
void platform_make_context(ucontext_t *context, void (*func)(void *, void *),
                           void *context1, void *context2);

#ifdef __cplusplus
}
#endif
