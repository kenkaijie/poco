.. SPDX-FileCopyrightText: Copyright contributors to the poco project.
.. SPDX-License-Identifier: MIT
==================
Navigating the API
==================

To get the best use out of the APIs, there are a few naming conventions used to help you
decide the best form.

Typically, each synchronisation primitive function will fit into 1 of 3 forms:

- a plain form,
- a ``*_no_wait`` form, and
- a ``*_from_isr`` form.

Some primitive structures have all 3, whereas some may only have 1 or 2.

For use within coroutines, a plain form or ``*_no_wait`` form can be used. ``*_no_wait``
functions will not yield back to the scheduler, but will still queue any required
events.

.. warning::

    Performing too many ``*_no_wait`` functions without yielding may overload the
    scheduler queue from processing. The limit on the number of functions is dependent
    on the available event buffer within the scheduler.

``*_from_isr`` calls should only be performed within an ISR. These will always return a
special code :cpp:enumerator:`result::RES_NOTIFY_FAILED`. This error should always be
caught as it indicates the scheduler has insufficient storage to queue up the event.


