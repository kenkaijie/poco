#
# Platform targets are separated by the PLATFORM option. Depending on the target
# compilation platform.
#
# SPDX-FileCopyrightText: Copyright contributors to the poco project.
# SPDX-License-Identifier: MIT

if (NOT "${POCO_CUSTOM_PLATFORM}" STREQUAL "")

    if (${POCO_CUSTOM_PLATFORM} STREQUAL "zephyr-armv7-m")
        add_subdirectory(zephyr-armv7-m)
    else()
        # User provided platform, they should add their platform sources to the 
        # poco::poco target.
    endif()

elseif (${UNIX})
    add_subdirectory(unix)
elseif (${WIN32})
    add_subdirectory(windows)
else()
    message(
        FATAL_ERROR
        "Unsupported platform: ${CMAKE_SYSTEM_NAME}. Check platforms folder for possibilities"
    )
endif()
