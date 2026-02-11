.. SPDX-FileCopyrightText: Copyright contributors to the poco project.
.. SPDX-License-Identifier: MIT

=======
Mutexes
=======

A mutex (MUTually EXclusive) is a coroutine specific synchronisation primitive. It is
used for managing access to shared resources such that only 1 coroutine can access the
resource at any given time.

The mutex has the following properties:

- Must be called within a coroutine only, ISRs, code executed outside the scheduler
  cannot use the mutex. For synchronisation is required for these cases, see
  :ref:`semaphore:Semaphores` instead.
- Can be called multiple times within the same coroutine.

This functionality is available from the specific header ``<poco/mutex.h>``.

Creating a Mutex
================

Mutexes are created by using either the static or dynamic constructors,
:cpp:func:`mutex_create_static` and :cpp:func:`mutex_create` respectively. 

Acquiring and Releasing
=======================

To use the mutex, it much first be acquired. If acquired, it guarantees that all other
coroutines attempting to acquire the mutex will be blocked until it is released.

.. warning::

    Failure to release the mutex will cause a deadlock, where all other coroutines will
    wait their defined timeout.

No Wait Variants
================

The mutex supoprts a no wait variant acquiring the mutex.

