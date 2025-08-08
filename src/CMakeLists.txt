# SPDX-FileCopyrightText: Copyright contributors to the poco project.
# SPDX-License-Identifier: MIT

target_sources(poco PRIVATE
    context.c
    coro.c
    event.c
    mutex.c
    queue.c
    scheduler.c
    semaphore.c
)

add_subdirectory(scheduler)
