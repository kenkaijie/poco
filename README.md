# poco - Pocket Coroutines

<img src="docs/_static/logo-transparent.svg" alt="poco" style="max-height:160px;margin-left: auto;margin-right: auto;display:block;"/>

![](https://github.com/kenkaijie/poco/actions/workflows/ubuntu.yml/badge.svg)
![](https://readthedocs.org/projects/poco-coro/badge/?version=latest)

This is a small toy project that produces a bare minimum cooperative task framework.

The objective here is to provide the minimum functionalities needed by a scheduler in
order to support general purpose applications on a bare metal device.

## Features

- Static task creation.
- Message queue.
- Self Managed Scheduling (roll your own scheduler)
- Basic Scheduling (Round Robin)

## WIP

- Basic Schedulers (Priority, Priority Round Robin)
- More Thread Primitives? (Events, Mutexes, Sempahores)
- ISR Support (Primitive support for ISR usage, scheduler support for ISR)
- Scheduler guidelines (tickless scheduler?)

### Extensions

- Building a framework for creating awaitable values? (This could just be using event flags.)

## Should I Use This?

Probably not, the goal of this project is mostly about learning and playing around.
There are more mature solutions in this space such as:

- Zephyr, and
- FreeRTOS,

that can probably be more suited for your projects.
