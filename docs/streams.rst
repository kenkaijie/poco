.. SPDX-FileCopyrightText: Copyright contributors to the poco project.
.. SPDX-License-Identifier: MIT

=======
Streams
=======

poco provides an API for sending bytes between ISR and coroutine, or between 2
coroutines.

Streams can only be used in situations where there is a single producer and single
consumer.

The typical use case for this is to queue up bytes from an ISR handling some hardware
protocol, such as UART, into a coroutine for handling.

.. note::

    Streams are a byte based messaging construct and should be used for streams of data
    as opposed to explicitly framed data. If you wish to pass framed data, consider
    using regular queues.

Stream functionality is available in the header ``<poco/stream.h>`` or the global header
``<poco/poco.h>``.

Writing
=======

Writing to a stream from a coroutine is done by using :cpp:func:`stream_send`. If
calling send from an ISR, use :cpp:func:`stream_send_from_isr` instead.

Coroutine based producers also have an additional option to wait until the queue empties
using :cpp:func:`stream_flush`. This is possible as we only have a single producer.

Reading
=======

Reading to a stream uses the equivalent receiving functions. When in a coroutine,
use :cpp:func:`stream_receive`.

The behaviour for receive is to wait until the timeout, or the given buffer is full. If
you do not have a need to wait the entire timeout, but are not sure how many bytes you
are getting, using the method :cpp:func:`stream_receive_up_to` provides a more
responsive read. This function will block if there is no data, but will return if there
is any number of bytes (up to the specific length).

When calling from an ISR, use :cpp:func:`stream_receive_from_isr`.

Length Checking
===============

The stream API also provides methods to fetch how many bytes are used
(:cpp:func:`stream_bytes_used`) and how many bytes are free
(:cpp:func:`stream_bytes_free`).

It is important to understand that the values provided by these are not exact values,
but represent **bounds**. This is because the other party involved may be performing
actions that will cause the returned value to no longer be correct.

As an example on the consumer.

- The consumer calling :cpp:func:`stream_bytes_used` is guaranteed the true value will be
  **greater than or equal** to the returned value, as the producer may have added more
  bytes which would increase the number of bytes used.
- The consumer calling :cpp:func:`stream_bytes_free` is guaranteed the true value will be
  **less than or equal** to the returned value, as the producer may have added more
  bytes which would decrease the number of free bytes.

The guarantees are the opposite when the functions are called from the producer.

- The producer calling :cpp:func:`stream_bytes_used` is guaranteed the true value will be
  **less than or equal** to the returned value.
- The producer calling :cpp:func:`stream_bytes_used` is guaranteed the true value will be
  **greater than or equal** to the returned value.

If called from neither the producer nor the consumer, the value is guaranteed to be
wrong.

No Wait Variants
================

The stream API also provides the no wait variants of send and receive.
