/*!
 * @file
 * @brief An example of using events to signal between coroutines.
 *
 * In this example, a producer task will emit a sequence of button press events.
 *
 * The press handler task is only interested in the long presses, and will ignore the
 * others.
 *
 */
#include <poco/coro.h>
#include <poco/event.h>
#include <poco/scheduler.h>
#include <poco/schedulers/round_robin.h>
#include <stdio.h>

#define CONSUMER_STOP (0xFFFFFFFF)
#define STACK_SIZE (1024)

CORO_STATIC_DEFINE(producer, STACK_SIZE);
CORO_STATIC_DEFINE(consumer, STACK_SIZE);

event_t button_event;

enum button_event_type {
    EVT_PRESS = (1 << 0),
    EVT_LONG_PRESS = (1 << 1),
    EVT_DOUBLE_PRESS = (1 << 2),
};

void producer_task(coro_t *coro, void *context) {
    event_t *event = (event_t *)context;
    coro_yield_delay(coro, 100);
    printf("Trigger press\n");
    event_set(coro, event, EVT_PRESS);
    coro_yield_delay(coro, 100);
    printf("Trigger press and double press\n");
    event_set(coro, event, EVT_PRESS | EVT_DOUBLE_PRESS);
    coro_yield_delay(coro, 100);
    printf("Trigger press and long press\n");
    event_set(coro, event, EVT_PRESS | EVT_LONG_PRESS);
    return;
}

void consumer_task(coro_t *coro, void *context) {
    event_t *event = (event_t *)context;
    flags_t flags =
        event_get(coro, event, EVT_LONG_PRESS, 0, true, PLATFORM_TICKS_FOREVER);
    if (flags & EVT_LONG_PRESS) {
        printf("Handling long press\n");
    }
    return;
}

int main() {

    coro_t *tasks[2] = {0};

    event_t *event = event_create_static(&button_event, 0);

    tasks[0] = coro_create_static(&producer_coro, producer_task, (void *)event,
                                  producer_stack, STACK_SIZE);
    tasks[1] = coro_create_static(&consumer_coro, consumer_task, (void *)event,
                                  consumer_stack, STACK_SIZE);

    round_robin_scheduler_t *scheduler = round_robin_scheduler_create(tasks, 2);

    if (scheduler == NULL) {
        printf("Failed to create scheduler\n");
        return -1;
    }

    scheduler_run((scheduler_t *)scheduler);

    return 0;
}
