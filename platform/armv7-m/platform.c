#include <poco/platform.h>
#include <stdarg.h>
#include <stddef.h>

// FIXME: this doesn't work
__attribute__ ((optimize ("omit-frame-pointer")))
static void trampoline(void)
{
  register ucontext_t *uc_link = NULL;

  asm volatile (
    "mov %0, r4\n"
    : // No output operands
    : "r" (uc_link) // Input operands
  );

  if (uc_link == NULL)
  {
    return;
  }

  setcontext(uc_link); // Restore the context of the link register
}

/*!
 * Goal is to save the machine's registers into the structured defined by the context.
 *
 * This is a superset of the Callee saved registers (R4-R8, R10, R11) as we might not
 * be at the end of a function, so we need to save everything.
 * 
 * We skip (R0-R3) as these are the registers used for function arguments and return
 * values of this function (getcontext), and thus do not need to be saved.
 */
int platform_get_context(ucontext_t * ucp)
{
  asm volatile (
    "add r1, %0, #0x1C\n" // offset to the machine context, skipping first 4 registers (12 + 16)
    "stmia r1, {r4-r12}\n" // Store r4 to r12 in memory
    "str sp, [%0, #0x40]\n" // Store stack pointer in ctx->r13 (offset 0xC + 0x34)
    "str lr, [%0, #0x18]\n" // Store link register in ctx->r3 (offset 0xC + 0xC)
    : // No output operands
    : "r" (ucp) // Input operands
  );
  return 0;
}

int platform_set_context(const ucontext_t * ucp)
{
  asm volatile (
    "add r14, %0, #0xC\n" // setup as address (r14 = &context->uc_mcontext)
    "ldmia r14, {r0-r12}\n" // restore all registers
    "ldr r13, [r14, #0x34]\n" // restore stack from ctx->r13

    "ldr r14, [r14, #0x38]\n" // restore link register from ctx->r14
    "mov pc, r3\n" // Branch to the link register (return)
    : // No output operands
    : "r" (ucp)
  );
  return 0;
}

int platform_swap_context(ucontext_t * ocp, const ucontext_t * ucp)
{
  // This is basically a get and set context in 1 big call.
  asm volatile (

    // Save current context
    "str sp, [%0, #0x40]\n" // Store stack pointer in ctx->r13 (offset 0xC + 0x34)
    "add r2, %0, #0x0C\n" // r2 = &context->r0
    "stmia r2, {r0-r12}\n" // Store rest of registers (r0-r12) in context
    "str lr, [%0, #0x18]\n" // Store link register in ctx->r3 (offset 0xC + 0xC)
    
    // Restore new context
    "add r14, %1, #0xC\n" // setup as address (r14 = &context->uc_mcontext)
    "ldmia r14, {r0-r12}\n" // restore all registers r3 is the PC
    "ldr r13, [r14, #0x34]\n" // restore stack

    "ldr r14, [r14, #0x38]\n" // restore link register
    // FIXME: we need to restore the PC to the value of R15 in the context, but we cannot
    // do this as we need to restore all registers first, so that information is lost?
    "mov pc, r3\n" // Branch to the link register (return)
    : // No output operands
    : "r" (ocp), "r" (ucp) // Input operands
  );
  return 0;
}

void platform_make_context(ucontext_t *ucp, void (*func)(void*, void*), void * context1, void * context2)
{
  // Prepopulate first 4 args with corotuine args needed.
  // Deviating from standard as I couldn't get va_args to work.
  ucp->uc_mcontext.r0 = (uintptr_t) context1;
  ucp->uc_mcontext.r1 = (uintptr_t) context2;
  ucp->uc_mcontext.r2 = (uintptr_t) NULL;
  ucp->uc_mcontext.r3 = (uintptr_t) func;

  // Set stack pointer to top of the stack as it is growing downwards.
	ucp->uc_mcontext.r13_sp = (uintptr_t) ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size;
	ucp->uc_mcontext.r15_pc = (uintptr_t) func;
	ucp->uc_mcontext.r4 = (uintptr_t) ucp->uc_link;
	ucp->uc_mcontext.r14_lr = (uintptr_t) &trampoline;
}
