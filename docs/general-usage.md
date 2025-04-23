# General Usage and Conventions

## Communication Primitives

poco supports a minimal set of communication primitives. These primitives support
communication between coroutines. It is worth noting that without interrupts, all
corotuines can share memory space. The primitives help better express concepts, as it
allows expression of tasks that are "waiting" for some input.

- `queue.h` for homogenous multi-byte elements.
- `event.h` for bit fields.

### ISR Calling Conventions

Calls within an interrupt context *MUST* use the `*_from_ISR()` functions to ensure
proper stack state.

As this is a cooperative scheduler, events triggered from an ISR cannot preempt the
exsiting running thread. Depending on the required performance, choosing a scheduler
algorithm that supports corotine priority will ensure a more timely response.
