
add_executable(sample_aggregate_wait sample_aggregate_wait.c consumer.c)
target_link_libraries(sample_aggregate_wait PRIVATE poco::poco)
