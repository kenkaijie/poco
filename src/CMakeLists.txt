

target_sources(poco PRIVATE 
    coro.c
    event.c
    queue.c
)

add_subdirectory(scheduler)
