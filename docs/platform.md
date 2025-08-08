<!-- 
SPDX-FileCopyrightText: Copyright contributors to the poco project.
SPDX-License-Identifier: MIT
-->
# Platform Porting

poco is a realtively bare bones library, as such, it requires little from the underlying
hardware to function. This page provides a small guide for platform implementors looking
to use poco on their specific platform.

The `platform.c/h` port is split into 3 sections, covering the minimal support required
to run the scheduler.

Note all symbols do not explicitly require to be functions. No assumptions will be made
that they are. In some platform samples, some of the required items are expressed as
macros.

poco includes platform information via the include `#include <poco/platform.h>`, ensure
the platform implementation exposes a header in that location in the compilers search
paths.

## Context Switching

This is the main API allowing coroutines to function. The context switch API is roughly
modelled around `ucontext` and borrows all the names.

The PC platform port (`unix`) just uses ucontext under the hood.

- `platform_context_t` should be a type definition pointing to an structure representing
  all required information to describe the CPU state at any given point.
- `platform_stack_t` should bne defined to the aligned type required for the stack. For
  maximum interop, stack sizes should be defined at an array of 32-bit values.
- `platform_get_context()` is used to save the current CPU state to the provided context
  for resumption at a later point.
- `platform_set_context()` sets the machine's context to the provided context. Note this
  does not return, as the context switch results in a jump.
- `platform_swap_context()` effectly performs a get operation, followed by a set
  operation.
- `platform_make_context()` modifies the input context to jump to the defined entrypoint.

## Hardware/Kernel Timing

Timing is required for implementation of yield delays. To remain as broad as possible,
the following definitions need to be present.

1. Arithmetic with ticks must allow for overflows, i.e TICK_MAX + 1 should equal 
2. Ticks must be a positive value.
3. Tick arithmetic is always performed as differences between 2 points. The value of the
   tick at any given point is not relevent.

- `platform_ticks_t` type defintion of the tick unit.
- `platform_get_monotonic_ticks()` get the current tick value.
- `platform_get_ticks_per_ms()` helps perform conversion between
  ticks and clock time.

## Critical Sections

For some communication primitives, in order to ensure correct operation in the presence
of interrupts, it sometimes is required to mask interrupts for a short duration to
ensure state updates remain "atomic" relative to the scheduler.

Execution of critical sections will uphold the following constraints:

1. These are always called in a pair (enter, exit).
2. These are always called within a single scope.

This is defined using an enter and exit operation.

- `platform_enter_critical_section()`
- `platform_exit_critical_section()`
