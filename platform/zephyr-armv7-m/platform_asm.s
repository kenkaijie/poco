// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief Cortex M4 Soft FP implementation of platform context switching for poco cooperative threading.
 *
 * @note As this is for software floating points, the FP registers are not preserved.
 */

  .syntax unified
  .cpu cortex-m4
  .fpu softvfp
  .thumb

.global  platform_swap_context
.global  platform_set_context
.global  platform_get_context

// Note this is just set and get (with slightly different registers)
//int platform_swap_context(PlatformContext *ocp (r0), const PlatformContext *ucp (r1)) {
platform_swap_context:
    // Save current context, the state to save is one where we are about to return.
	add   	r0, r0, #0x1C		// r0 = &ctx->r4 (sizeof(uc_link)=4, sizeof(stack_descriptor)=12)
	stmia  	r0, {r4-r12}		// Store r4 to r12 in context
	str    	sp, [r0, #0x24]		// Store stack pointer in ctx->r13
	str    	lr, [r0, #0x28]		// Store link register in ctx->r14
	movs   	r2, #0              // Store a 0 in r0 to return 0.
	str    	r2, [r0, #-0x10]	// store r0 as the return value

    // Restore new context
	add 	r14, r1, #0xC		// r14 = &ctx->r0
    ldmia 	r14, {r0-r12}		// restore all registers
    ldr 	r13, [r14, #0x34]	// restore stack
    ldr 	r14, [r14, #0x38]	// restore link register

    bx		lr				// Manual return back.


/* int platform_set_context(const PlatformContext *ucp (r0)) */
platform_set_context:
	add 	r14, r0, #0xC		// r14 = &ctx->r0
    ldmia 	r14, {r0-r12}		// restore all registers
    ldr 	r13, [r14, #0x34]	// restore stack
    ldr 	r14, [r14, #0x38]	// restore link register

    // even if the API says an int return, we dont set the return value here as we
    // will just return the restored value.
    bx 		lr				// Manual return back.


/* int platform_get_context(PlatformContext *ucp (r0))  */
platform_get_context:
    // Save current context, the state to save is one where we are about to return.
	add   	r0, r0, #0x1C		// r0 = &ctx->r4 (sizeof(uc_link)=4, sizeof(stack_descriptor)=12)
	stmia  	r0, {r4-r12}		// Store r4 to r12 in context
	str    	sp, [r0, #0x24]		// Store stack pointer in ctx->r13
	str    	lr, [r0, #0x28]		// Store link register in ctx->r14
	movs   	r2, #0              // Store a 0 in r0 to return 0.
	str    	r2, [r0, #-0x10]	// store r0 as the return value

	movs	r0, #0				// return 0

	bx		lr				// Manual return back.
