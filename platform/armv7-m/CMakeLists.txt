add_library(poco_platform)
add_library(poco::platform ALIAS poco_platform)
target_include_directories(poco_platform PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_sources(poco_platform PRIVATE platform.c)
