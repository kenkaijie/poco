
add_executable(sample_hello_world sample_hello.c)
target_link_libraries(sample_hello_world PRIVATE poco::poco)
