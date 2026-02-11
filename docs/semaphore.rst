.. SPDX-FileCopyrightText: Copyright contributors to the poco project.
.. SPDX-License-Identifier: MIT

==========
Semaphores
==========

For general purpose resource management and synchronisation, a semaphore primitive is
provided. poco provides both binary and counting semaphores for use within coroutines
and ISRs.

Sempahores are a generalisation of mutexes and can be used for synchronisation between
coroutines and ISRs, or for managing access to a shared resource with a specific number
of instances.

For synchronisation between coroutines only, consider using :ref:`mutex:Mutexes`
instead.

This functionality is available from the specific header ``<poco/semaphore.h>``.

Creating a Semaphore
====================

Semaphores can either be created statically or dynamically. This API provides the
``*_create`` and ``*_create_static`` variants for both types of semaphores.

- For binary semaphores, you can use :cpp:func:`semaphore_create_binary` or
  :cpp:func:`semaphore_create_binary_static`.
- For counting semaphores, you can use :cpp:func:`semaphore_create` or
  :cpp:func:`semaphore_create_static`.

Acquiring and Releasing
=======================

Sempahores can be acquired and released using the ``*_acquire`` and ``*_release``
functions. The exact function depends on the execution context of the caller.

- For coroutines, use :cpp:func:`semaphore_acquire` and :cpp:func:`semaphore_release` or
  the no wait variant :cpp:func:`semaphore_acquire_no_wait`. 
- For ISRs and code executed outside the scheduler, use
  :cpp:func:`semaphore_acquire_from_isr` and :cpp:func:`semaphore_release_from_isr`.

.. warning::

    Failure to release the semaphore may cause a deadlock, where all other coroutines
    will wait their defined timeout.
