#include <poco/queue.h>
#include <poco/intercoro.h>
#include <string.h>


queue_t * queue_create_static(queue_t* queue, size_t num_elements, size_t element_size, uint8_t * buffer)
{
    queue->buffer = buffer;
    queue->count = 0;
    queue->read_idx = 0;
    queue->write_idx = 0;
    queue->element_size = element_size;
    queue->max_items = num_elements;
    return queue;
}

size_t queue_count(queue_t * queue)
{
    return queue->count;
}

bool queue_full(queue_t * queue)
{
    return queue_count(queue) == queue->max_items;
}

bool queue_empty(queue_t * queue)
{
    return queue_count(queue) == 0;
}

error_t queue_put(coro_t * coro, queue_t * queue, void * item)
{
    while(queue_full(queue))
    {
        // Wait for space to be available
        
        coro->event_sinks[0].type = CORO_EVTSINK_QUEUE_NOT_FULL;
        coro->event_sinks[0].params.queue = queue;
        coro->event_sinks[1].type = CORO_EVTSINK_NONE;

        coro_yield_with_signal(coro, CORO_SIG_WAIT);
    }

    memcpy(&queue->buffer[queue->write_idx * queue->element_size], item, queue->element_size);
    queue->write_idx = (queue->write_idx + 1) % queue->max_items;
    queue->count++;

    coro->event_source.type = CORO_EVTSRC_QUEUE_PUT;
    coro->event_source.params.queue = queue;
    coro_yield_with_signal(coro, CORO_SIG_NOTIFY);

    return RET_OK;
}

error_t queue_get(coro_t * coro, queue_t * queue, void * item)
{
    while(queue_empty(queue))
    {
        coro->event_sinks[0].type = CORO_EVTSINK_QUEUE_NOT_EMPTY;
        coro->event_sinks[0].params.queue = queue;
        coro->event_sinks[1].type = CORO_EVTSINK_NONE;

        coro_yield_with_signal(coro, CORO_SIG_WAIT);
    }

    memcpy(item, &queue->buffer[queue->read_idx * queue->element_size], queue->element_size);
    queue->read_idx = (queue->read_idx + 1) % queue->max_items;
    queue->count--;

    coro->event_source.type = CORO_EVTSRC_QUEUE_GET;
    coro->event_source.params.queue = queue;
    coro_yield_with_signal(coro, CORO_SIG_NOTIFY);

    return RET_OK;
}
