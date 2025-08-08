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

Task Creation
=============

Task creation has 2 main steps, creation of the task object, and assigning the task to a
scheduler. In most cases, every application has a single scheduler.

For statically defined tasks, a convinience macro :c:macro:`CORO_STATIC_DEFINE` has
been provided. Here we use this to create the 2 tasks we will need.

.. literalinclude:: ../../samples/hello-world/sample_hello.c
    :language: c
    :lines: 17, 18

Once created, they can be initialised in the main function.

The second step involves creation of the scheduler. poco provides some basic scheduler
implementations to use. In this example, we will use :cpp:struct:`round_robin_scheduler`
as it provided the behaviour we require (alternating between all tasks).

Every scheduler implements a generic scheduler interface to allow for implementations
to support a wider variety of schedulers if needed.

.. literalinclude:: ../../samples/hello-world/sample_hello.c
    :language: c
    :lines: 35-53

We use ``NULL`` for the user context, as they are not needed in this tutorial. More
complex applications may use this to pass around pointers to queue and other constructs.


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
    :lines: 19-25

The key line is where the yield takes place. This instructs the scheduler to pause the
current execution of the function, and move on to the next task. When the scheduler
returns to this function, it will resume where it left off.

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
