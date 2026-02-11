.. SPDX-FileCopyrightText: Copyright contributors to the poco project.
.. SPDX-License-Identifier: MIT

==================
Navigating the API
==================

poco aims to provide all the bare neccessities for a coroutine library. At a glance, it
consists of the basic coroutine operations, and a minimal set of corotuine and scheduler
aware primitives for synchronisation and communication.

The table below provides a quick reference to the available primitives and their
functions. These are organised based on the execution context they are designed for.
Details about the specific functions and their usage can be found in the respective
sections.

.. list-table:: Primitive API Overview
    :header-rows: 1
    
    * -
      - Creation
      - Coroutine
      - ISR and External Code
    * - :ref:`events:Events`
      - :cpp:func:`event_create`
        :cpp:func:`event_create_static`
        :cpp:func:`event_free`
      - :cpp:func:`event_set`
        :cpp:func:`event_set_no_wait`
        :cpp:func:`event_get`
        :cpp:func:`event_get_no_wait`
      - :cpp:func:`event_set_from_isr`
        :cpp:func:`event_get_from_isr`
    * - :ref:`queues:Queues`
      - :cpp:func:`queue_create`
        :cpp:func:`queue_create_static`
        :cpp:func:`queue_free`
      - :cpp:func:`queue_put`
        :cpp:func:`queue_put_no_wait`
        :cpp:func:`queue_get`
        :cpp:func:`queue_get_no_wait`
      - :cpp:func:`queue_put_from_isr`
        :cpp:func:`queue_get_from_isr`
    * - :ref:`streams:Streams`
      - :cpp:func:`stream_create`
        :cpp:func:`stream_create_static`
        :cpp:func:`stream_free`
      - :cpp:func:`stream_send`
        :cpp:func:`stream_send_no_wait`
        :cpp:func:`stream_receive`
        :cpp:func:`stream_receive_up_to`
        :cpp:func:`stream_receive_no_wait`
      - :cpp:func:`stream_send_from_isr`
        :cpp:func:`stream_receive_from_isr`
    * - :ref:`mutex:Mutexes`
      - :cpp:func:`mutex_create`
        :cpp:func:`mutex_create_static`
        :cpp:func:`mutex_free`
      - :cpp:func:`mutex_acquire`
        :cpp:func:`mutex_acquire_no_wait`
        :cpp:func:`mutex_release`
      - N/A
    * - :ref:`semaphore:Semaphores`
      - :cpp:func:`semaphore_create_binary`
        :cpp:func:`semaphore_create_binary_static`
        :cpp:func:`semaphore_create`
        :cpp:func:`semaphore_create_static`
        :cpp:func:`semaphore_free`
      - :cpp:func:`semaphore_acquire`
        :cpp:func:`semaphore_acquire_no_wait`
        :cpp:func:`semaphore_release`
      - :cpp:func:`semaphore_acquire_from_isr`
        :cpp:func:`semaphore_release_from_isr`

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

Some primitives may have all 3, whereas some may only have 1 or 2.

The plain form is designed for use within coroutines.

For primitives that support use within an ISR, the ``*_from_isr`` should be used. The
ISR variants use a special scheduler based notification system to report events. These
variants will return a special code :cpp:enumerator:`result::RES_NOTIFY_FAILED` when the
scheduler has insufficient storage in its event buffer to queue up the event.

When creating a scheduler, ensure the event buffer is set up to handle the maximum
number of interrupts between the worst case coroutine yield.

The additional form ``*_no_wait`` is provided for advanced use cases and are designed
for coroutine use where yielding is not desired. As no yielding is performed, these
consume the same scheduler event buffer as the ``*_from_isr`` variants. Care must be
taken ensure sufficient slots are available for the expected use case.
