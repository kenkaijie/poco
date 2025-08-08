<!-- 
SPDX-FileCopyrightText: Copyright contributors to the poco project.
SPDX-License-Identifier: MIT
-->
# poco - Pocket Coroutines

<img src="docs/_static/logo-transparent.svg" alt="poco" style="max-height:160px;margin-left: auto;margin-right: auto;display:block;"/>

[![](https://github.com/kenkaijie/poco/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/kenkaijie/poco/actions/workflows/ubuntu.yml)
[![](https://readthedocs.org/projects/poco-coro/badge/?version=latest)](https://poco-coro.readthedocs.io/en/latest/)

This is a small toy project that produces a bare minimum cooperative task framework.

The objective is to provide a minimal framework for bare metal devices.

## Nutshell

Include poco.

```c
#include <poco/poco.h>
```

Define a coroutine.

```c
// Prints "hello world!" every 1 second.
static void my_coroutine(void *context) {
    for (;;) {
        printf("hello world!\n");
        coro_yield_delay(1000);
    }
}
```

Attach it to a scheduler and run.

```c
int main(int argc, char *argv[]) {
    // ...

    coro_t *tasks[1] = {
        coro_create(my_coroutine, NULL, DEFAULT_STACK_SIZE)
        /* add other tasks */
    };

    // ...

    scheduler_t *scheduler = round_robin_scheduler_create(tasks, 1);

    /* Will never return */
    scheduler_run(scheduler);
}
```

## Features

- Static task creation.
- Message queue.
- Self Managed Scheduling (roll your own scheduler)
- Basic Scheduling (Round Robin)
- Runnable on POSIX hosts

## WIP

- Basic Schedulers (Priority, Priority Round Robin)
- More Thread Primitives? (Events, Mutexes, semaphores)
- ISR Support (Primitive support for ISR usage, scheduler support for ISR)
- Scheduler guidelines (tickless scheduler?)
- Dynamic coroutine support (adding coroutines at runtime).

### Extensions

- Building a framework for creating awaitable values? (This could just be using event flags.)

## Should I Use This?

Probably not, the goal of this project is mostly about learning and playing around.
There are more mature solutions in this space such as:

- Zephyr, and
- FreeRTOS,

that can probably be more suited for your projects.
