// SPDX-FileCopyrightText: Copyright contributors to the poco project.
// SPDX-License-Identifier: MIT
/*!
 * @file
 * @brief An example of using events to signal between coroutines.
 *
 * In this example, a producer task will emit a sequence of button press events.
 *
 * The press handler task is only interested in the long presses, and will ignore the
 * others.
 */

#include <poco/coro.h>
#include <poco/event.h>
#include <poco/scheduler.h>
#include <poco/schedulers/round_robin.h>
#include <stdio.h>

#define CONSUMER_STOP (-1)
#define STACK_SIZE (DEFAULT_STACK_SIZE)

Coro producer_coro = {0};
PlatformStackElement producer_stack[STACK_SIZE] = {0};

Coro consumer_coro = {0};
PlatformStackElement consumer_stack[STACK_SIZE] = {0};

Event button_event;

enum button_event_type {
    EVT_PRESS = (1 << 0),
    EVT_LONG_PRESS = (1 << 1),
    EVT_DOUBLE_PRESS = (1 << 2),
};

void producer_task(void *context) {
    Event *event = context;
    coro_yield_delay(100);
    printf("Trigger press\n");
    event_set(event, EVT_PRESS);
    coro_yield_delay(100);
    printf("Trigger press and double press\n");
    event_set(event, EVT_PRESS | EVT_DOUBLE_PRESS);
    coro_yield_delay(100);
    printf("Trigger press and long press\n");
    event_set(event, EVT_PRESS | EVT_LONG_PRESS);
}

void consumer_task(void *context) {
    Event *event = context;
    Flags const flags = event_get(event, EVT_LONG_PRESS, EVENT_FLAGS_MASK_ALL, true,
                                  PLATFORM_TICKS_FOREVER);
    if (flags & EVT_LONG_PRESS) {
        printf("Handling long press\n");
    }
}

int main(void) {

    Coro *tasks[2] = {0};

    Event *event = event_create_static(&button_event, 0);

    tasks[0] = coro_create_static(&producer_coro, producer_task, event, producer_stack,
                                  STACK_SIZE);
    tasks[1] = coro_create_static(&consumer_coro, consumer_task, event, consumer_stack,
                                  STACK_SIZE);

    Scheduler *scheduler =
        round_robin_scheduler_create(tasks, sizeof(tasks) / sizeof(tasks[0]));

    if (scheduler == NULL) {
        printf("Failed to create scheduler\n");
        return -1;
    }

    scheduler_run(scheduler);

    return 0;
}
