cmake_minimum_required(VERSION 3.19) # 3.19 for preset support

project(poco LANGUAGES C ASM)

set(POCO_PLATFORM "unix" CACHE STRING "Platform to compile for")

add_subdirectory(platform)
add_subdirectory(samples)

add_library(poco)
add_library(poco::poco ALIAS poco)
target_link_libraries(poco PUBLIC poco::platform)
target_include_directories(poco PUBLIC include)
add_subdirectory(src)

# Add testing

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(tests)
endif()
