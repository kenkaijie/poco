

target_sources(poco PRIVATE 
    coro.c
    event.c
    mutex.c
    queue.c
)

add_subdirectory(scheduler)
