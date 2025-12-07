.. SPDX-FileCopyrightText: Copyright contributors to the poco project.
.. SPDX-License-Identifier: MIT
======
Events
======

The event primitive allows for a lightweight one-way communication mechanism. Each event
is divided into individual bits, where a listener can wait for a change in any of the
bits based on a predefined mask.

.. warning::

    Events cannot be read from an ISR.

Each event object should have only one consumer. Multiple producers are allowed.

Event functionality is available from the specific ``<poco/event.h>`` or the global
``<poco/poco.h>`` headers.

Setting Events
==============

Events can be set from a coroutine using :cpp:func:`event_set`. From an ISR, the
equivalent :cpp:func:`event_set_from_isr` is used instead.

Flags can be set prior to the consumer listening to the event. If set prior to reading,
the first listen may immediately trigger the event without a yield.

Getting Events
==============

Events can only be fetched from a coroutine using :cpp:func:`event_get`. When fetching,
a listening mask is specified, which allows the consumer to wait on a specific
combination of bits.

Once triggered, the clear mask is then used to reset any bits as required.
