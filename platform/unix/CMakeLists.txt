add_library(poco_platform INTERFACE)
add_library(poco::platform ALIAS poco_platform)
target_include_directories(poco_platform INTERFACE ${CMAKE_CURRENT_LIST_DIR})
