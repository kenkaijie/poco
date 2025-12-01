// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief ARM M4 Specific platform code.
 */

#include <poco/platform.h>
#include <stdarg.h>
#include <stddef.h>

// We modify the context to think it originates from the function provided. That way the
// next time we resume this context, it will magically appear inside.
void platform_make_context(PlatformContext *ucp, void (*entrypoint)(void *, void *),
                           void *coro, void *user_context) {
    // Pre-populate first 4 with arguments used within the entry point (+ some others).
    // This is okay as we know the first 2 will be the correct values.

    // Deviating from standard as I couldn't get va_args to work.
    ucp->uc_mcontext.r0 = (uintptr_t)coro;
    ucp->uc_mcontext.r1 = (uintptr_t)user_context;

    // Set stack pointer to top of the stack as it is growing downwards.
    ucp->uc_mcontext.r13_sp = (uintptr_t)ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size;
    ucp->uc_mcontext.r14_lr = (uintptr_t)entrypoint;
}
