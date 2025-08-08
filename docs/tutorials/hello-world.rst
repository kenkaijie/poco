.. SPDX-FileCopyrightText: Copyright contributors to the poco project.
.. SPDX-License-Identifier: MIT
====================
hello-world Tutorial
====================

This tutorial shows the basic operation of poco and showcases the scheduler's operation.
The goal here is to have a program that prints the string ``Hello World!`` 5 times, then
exits.

In this tutorial, you will be exposed to the following poco concepts.

1. Task creation.
2. Scheduler use.
3. Yielding.

.. note::

    The hello world sample is located within the project under ``samples/hello-world``.
    If using CMake, the target ``sample_hello_world`` builds this example.

The full program snippet can be found at the bottom of this page.

Overview
========

The operation of this program uses 2 tasks. A task for printing the string ``Hello ``, 
which we will call the ``hello_task``, and the other for printing ``World!\n``, which
is will be called the ``world_task``.

The execution plan can be summarised below.

.. uml::
    :caption: Execution plan for tasks.
    :align: center

    @startuml

    concise "hello" as H
    concise "world" as W
    scale 1 as 100 pixels
    hide time-axis

    @0
    H is "Hello"
    W is {-}

    @1
    H is {-}
    W is "World!"
    H-> W: yield()

    @2
    H is "Hello"
    W is {-}
    W-> H: yield()

    @3
    H is {-}
    W is "World!"
    H-> W: yield()

    @enduml

Defining the Tasks
==================

Thinking about the tasks in isolation, each task will either print the value ``Hello``
or ``World!\n`` the console, then wait until the next task.

Each task function has a user provided ``context``. This context allows programs to pass
in references to items the task requires to operate. In this tutorial, the additional
context is not required.

The sample below shows the implementation for the task printing ``Hello``. This can be
implemented using normal flow control, and does not require any special handling.

.. literalinclude:: ../../samples/hello-world/sample_hello.c
    :language: c
    :lines: 21-28

The key line is where the yield takes place. This instructs the scheduler to pause the
current execution of the function, and move on to the next task. When the scheduler
returns to this function, it will resume where it left off.

Coroutine Creation
==================

Each function that will be run as a task will need to be associated with a single
coroutine structure. These can be defined in the program's ``main``.

Each of the calls to ``coro_create`` will create a coroutine structure used by the
scheduler. This includes all the necessary information to allow coroutines to pause and
resume execution.

.. literalinclude:: ../../samples/hello-world/sample_hello.c
    :language: c
    :lines: 41-44

Each call takes in 3 arguments, the function this coroutine will run, a user provided
context, and the size of stack to allocate to this coroutine.

As this is a simple example, we can use the platform provided default stack sizes, as
well as setting the context to ``NULL``. Any context provided here will be passed to
the function signature.

The second step involves creation of the scheduler. poco provides some basic scheduler
implementations to use. In this example, we will use :cpp:struct:`round_robin_scheduler`
as it provided the behaviour we require (alternating between all tasks).

Every scheduler implements a generic scheduler interface to allow for implementations
to support a wider variety of schedulers if needed. Once created, we can run using the
:cpp:func:`scheduler_run` method to run the freshly created scheduler.

.. literalinclude:: ../../samples/hello-world/sample_hello.c
    :language: c
    :lines: 46-54

Running the Program
===================

Execution of the chosen scheduler always begins with the first task in the list. In this
example, it would be the hello task.

This results in the printouts to the console.

.. literalinclude:: ../../tests/test_sample_hello_world.expected
    :language: text

Full Reference
==============

.. literalinclude:: ../../samples/hello-world/sample_hello.c
    :language: c
