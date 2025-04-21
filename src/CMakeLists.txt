

target_sources(poco PRIVATE 
    coro.c
    queue.c
)

add_subdirectory(scheduler)
