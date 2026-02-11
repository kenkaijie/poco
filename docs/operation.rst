.. SPDX-FileCopyrightText: Copyright contributors to the poco project.
.. SPDX-License-Identifier: MIT

==================================
Scheduler and Coroutine Operation
==================================

poco uses a event broadcasting system to propagate signals between coroutines. The
scheduler acts as the broker and passes all queued signals it has between coroutine
execution.

.. note::

    These signals are distinct from OS signals.

This messaging between coroutines is how poco implements coroutine suspension.

Coroutine Operational States
============================

Each coroutine maintains its own internal read-only state. This indicates to the
scheduler which coroutines are ready to execute and which ones are waiting on an event.

.. uml::
    :caption: Possible coroutine states and transitions.
    :align: center

    @startuml

    state Ready
    state Running
    state Waiting
    state Finished

    [*] --> Ready : create

    Ready --> Running : coro_resume()

    Running --> Ready : coro_yield()
    Running --> Waiting : coro_yield(SIG_WAIT)

    Waiting --> Ready: coro_notify(), if event matches

    Running --> Finished : return

    Finished --> [*]

    @enduml

Communication with the Scheduler
================================

When a coroutine yields back control to the scheduler, it returns with a particular
signal the scheduler should action. These actions are found in
:cpp:enum:`coro_signal`.

Event Sinks and Sources
-----------------------

Internally, coroutines contain a single event source, and up to two event sinks. This
ensures the coroutine can suspend on an event, as well as an optional timeout value (if
using timeout aware APIs).

If the coroutine yields with :cpp:enumerator:`CoroSignal::CORO_SIG_NOTIFY` or
:cpp:enumerator:`CoroSignal::CORO_SIG_NOTIFY_AND_WAIT`, the scheduler must add the
notified event to its list of pending events.

Matching between event sinks and sources is done via the signal type. Most events have
a 1:1 relationship between the event source and the corresponding event sink.

A typical sequence is shown below for a coroutine informing the scheduler that it is
waiting for an event. Here we show coroutine A waiting on the
:cpp:enumerator:`CoroEventSinkType::CORO_EVTSINK_DELAY`.

.. uml::
    :caption: Example of coroutine suspension and unblock via the event primitive.
    :align: center

    @startuml

    Scheduler -> CoroutineA: coro_resume()
    activate CoroutineA
        ...
        CoroutineA -> CoroutineA: setup event sink (CORO_EVTSRC_EVENT_GET)
        Scheduler <-- CoroutineA: coro_yield(CORO_SIG_WAIT)
    deactivate CoroutineA

    Scheduler -> CoroutineB: coro_resume()
    activate CoroutineB
        ...
        CoroutineB -> CoroutineB: setup event source (CORO_EVTSRC_EVENT_SET)
        Scheduler <-- CoroutineB: coro_yield(CORO_SIG_NOTIFY)
    deactivate CoroutineB

    Scheduler -> CoroutineA: coro_notify(CORO_EVTSRC_EVENT_SET)

    @enduml
