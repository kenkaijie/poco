# SPDX-FileCopyrightText: Copyright contributors to the poco project.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.24) # 3.24 for FetchContent FIND_PACKAGE_ARGS

project(poco LANGUAGES C ASM)

# Options
set(
    POCO_PLATFORM
    "unix"
    CACHE
    STRING
    "Set the underlying platform. Default: unix. Values: {unix, custom}"
)

option(
    POCO_BUILD_TESTS
"\
Enable building tests and test infrastructure. Default: ${PROJECT_IS_TOP_LEVEL}.\
Values: { ON, OFF }.\
"
    ${PROJECT_IS_TOP_LEVEL}
)

option(
    POCO_BUILD_SAMPLES
"\
Enable building examples, not if building samples and enabling testing, additional\
sample tests are also enabled. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }.\
"
    ${PROJECT_IS_TOP_LEVEL}
)

add_library(poco)
add_library(poco::poco ALIAS poco)

set_target_properties(
    poco
    PROPERTIES
        VERIFY_INTERFACE_HEADER_SETS ON
        EXPORT_NAME poco
        C_STANDARD 99
)

add_subdirectory(include)
add_subdirectory(platform)
add_subdirectory(src)

# Add samples
if(POCO_BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

# Add testing
if(POCO_BUILD_TESTS)
    include(CTest)
    include(FetchContent)

    FetchContent_Declare(
        cmocka
        GIT_REPOSITORY https://gitlab.com/cmocka/cmocka.git
        GIT_TAG cmocka-1.1.5
        FIND_PACKAGE_ARGS 1.1.5
    )

    set(UNIT_TESTING OFF CACHE BOOL "")
    set(WITH_EXAMPLES OFF CACHE BOOL "")

    FetchContent_MakeAvailable(cmocka)

    add_subdirectory(tests)
endif()

# Installation Sets
include(GNUInstallDirs)

install(
    TARGETS poco COMPONENT poco
    EXPORT poco
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FILE_SET HEADERS
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export Sets
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/poco-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/poco-config.cmake
    INSTALL_DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/poco
    PATH_VARS PROJECT_NAME
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/poco-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/poco
    COMPONENT poco
)

install(
    EXPORT poco
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/poco
    NAMESPACE poco::
    FILE poco-targets.cmake
    COMPONENT poco
)
