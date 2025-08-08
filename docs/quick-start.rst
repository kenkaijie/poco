.. SPDX-FileCopyrightText: Copyright contributors to the poco project.
.. SPDX-License-Identifier: MIT
===========
Quick Start
===========

This guide provides small snippets on how to use poco.

The activities generally fall into the following steps.

1. Pick and initialise a scheduler.
2. Create tasks.
3. Run the scheduler in the main loop.

poco can be included either as a single header or as individual components. The examples
provided as part of this library showcase both styles.

Picking a Scheduler
===================

The scheduler determines which coroutine is run at each context switch. Every coroutine
is run within the context of a scheduler.

Coroutine/Task Creation
=======================

Tasks are the execution units within poco. You can group the program's activities in
the form of a coroutine and the poco scheduler will run in along side all other defined
coroutines.

.. literalinclude:: ../samples/queue/sample_queue.c
    :language: c
    :lines: 22-33
    :caption: A sample task function.

Once the function has been defined, it can be attached to a coroutine via
:cpp:func:`coro_create`.

.. literalinclude:: ../samples/queue/sample_queue.c
    :language: c
    :lines: 55-56
    :dedent: 4

To pause a coroutine, simply yield control back to the scheduler using the provided
yield commands. As this is a coroutine library, yielding back to the scheduler is under
control by the developer (with the exception of communication primitives).

.. note:: 
    
    The frequency and duration between yields directly effects the responsiveness of the
    program. It is best to yield often to ensure the best responsiveness.

- :cpp:func:`coro_yield` to yield.
- :cpp:func:`coro_yield_delay` to yield and sleep the task for a period of time.

poco's built-in communication primitives also perform yields, such as when putting items
in a :cpp:struct:`queue`.

If coroutines need to be sequential, :cpp:func`coro_join` can be used to ensure a
particular coroutine runs after another.

Scheduler
=========

TODO

Running
=======

TODO
