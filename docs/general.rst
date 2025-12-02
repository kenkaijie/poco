.. SPDX-FileCopyrightText: Copyright contributors to the poco project.
.. SPDX-License-Identifier: MIT
==================
Navigating the API
==================

To get the best use out of the APIs, there are a few naming conventions used to help
navigate the available functions.

Function Variants
=================

Synchronization primitives (``queues``, ``semaphores``, ``events``, etc.) expose up to
3 variants of each operation. These are designed to apply to a variety of execution
contexts.

.. danger::

    Incorrect usage of each variants may yield undefined or unexpected behaviour. Ensure
    the variant in use is correct for your specific execution context.

These include 2 general use forms, and a special form:

- a plain form,
- a ``*_from_isr`` form, and
- a ``*_no_wait`` form.

Some primitive may have all 3, whereas some may only have 1 or 2.

The plain form is the most straight forward, and should be used within any coroutine.
These yield the calling coroutine properly and ensures overall system performance.

For primitives that support use within an ISR, the ``*_from_isr`` should be used
instead. ISR variants can return a special code
:cpp:enumerator:`result::RES_NOTIFY_FAILED`. This error indicates the scheduler has
insufficient storage in its event buffer to queue up the event.

When creating a scheduler, ensure the event buffer is set up to handle the maximum
number of interrupts between the worst case coroutine yield.

No Yield Variants
=================

The additional form ``*_no_wait`` is provided for advanced use cases and should
generally be avoided. Calling these will use the scheduler notification system (instead
of the coroutine notification). Improper use may cause missed events that can stall or
deadlock the scheduler.

.. warning::

    Performing too many ``*_no_wait`` functions without yielding may overload the
    scheduler queue from processing. The limit on the number of functions is dependent
    on the configured event buffer size within the scheduler.
