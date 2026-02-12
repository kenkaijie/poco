==============
Extending poco
==============

There will be instances where the provided tools are simply not sufficient for your use
case. poco provides extension points to modify the behaviour without affecting the core.

.. danger::

    Extending poco is an advanced topic and should be approached with caution. It is
    recommended to have a good understanding of the core concepts and design of poco
    before attempting to extend it.

Custom Schedulers
=================

TODO

Custom Primitives
=================

.. warning::

    Custom primitives are an advanced feature and should rarely be needed. Consider
    carefully whether this is the right choice. In many cases, a composition of existing
    primitives may be sufficient to achieve the desired functionality.

To facilitate the development of custom primitives, poco provides a set of generic
source and sinks. These are the ``CORO_EVTSRC_CALLABLE`` and ``CORO_EVTSNK_CALLABLE``
event types, which can be used to create custom event sources and sinks.

When a coroutine blocks with the callable event sink, schedulers should match with the
same callable event source type.

Each callable event source provides 3 fields, a magic number code, a point to a source
instance, and a function pointer to a callback function.

- The magic number is used to classify the type of primitive. Ideally each primitive
  type within your system will have a unique magic number.
- A subject pointer, this points at the instance of the primitive causing the block.
- The callback function to decide if the coroutine can be unblocked. If not set, this
  will cause the coroutine to unblock if the subjects of the sink and source match.

.. danger::

    Development of trigger fuctions should be done with care. The trigger function will
    be called within the scheduler's execution context. Any heavy processing may
    significantly impact the performance of all corotuines within the system. It is
    recommended to keep trigger functions as lightweight as possible.

    Avoid calling other primitives here. Ideally trigger functions are purely
    functional and should only depend on the inputs provided.
