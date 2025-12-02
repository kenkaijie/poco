.. SPDX-FileCopyrightText: Copyright contributors to the poco project.
.. SPDX-License-Identifier: MIT
======
Queues
======

poco provides a coroutine aware general purpose bounded length queuing primitive.
This is the main mechanism for data transmissions between coroutines.

Queues are a multi-consumer multi-producer data structure. Usage includes sending data
between coroutines and sending data between ISRs and coroutines.

This functionality is available from the specific ``<poco/queue.h>`` or the global
``<poco/poco.h>`` headers.

Creating a Queue
================

To create a queue, either the dynamic constructor :cpp:func:`queue_create` or the static
constructor :cpp:func:`queue_create_static` can be used. In both cases, creating a
queue requires the caller know the maximum number of items and the size of each item.

The dynamically created queue can be freed using :cpp:func:`queue_free`, which handles
all internally allocated items too.

Putting Items
=============

Any item placed in the queue is copied into the queue's internal buffer. Once an item
has been queued, it does not need to be preserved in the caller's scope.

.. warning::

    Queues perform a shallow copy when putting and getting items.

Use :cpp:func:`queue_put` to place an item in the queue. If within an ISR, use the
ISR aware variant :cpp:func:`queue_put_from_isr`.

Getting Items
=============

Items obtained from the queue are removed from the queue once extracted. Similar to the
put functions, :cpp:func:`queue_get` can be used to fetch an item from the queue, or the
ISR aware variant :cpp:func:`queue_get_from_isr`.

Inspecting the Length
=====================

The queue's used length can also be inspected. Care is to be taken when using these
functions, as they may not be accurate depending on the types of producers and
consumers using the queue.

For example, an ISR producer may cause the :cpp:func:`queue_item_count` value to be
invalid as the function may return a count lower than the true count.

Raw Functions
=============

To facilitate general use of queues, the raw API is also available using the header
``<poco/queue_raw.h>``. This special header exposes the non coroutine aware base API for
use in specialised applications.

No Wait Variants
================

The queue API also provides the no wait variants of put and get.
