#
# Platform targets are separated by the PLATFORM option. Depending on the target
# compilation platform.
#
# SPDX-FileCopyrightText: Copyright contributors to the poco project.
# SPDX-License-Identifier: MIT

if (${POCO_PLATFORM} STREQUAL "unix")
    add_subdirectory(unix)
elseif (${POCO_PLATFORM} STREQUAL "zephyr-armv7-m")
    add_subdirectory(zephyr-armv7-m)
elseif (${POCO_PLATFORM} STREQUAL "custom")
    # User provided platform, they should add their platform sources to the 
    # poco::platform target.
else()
    message(
        FATAL_ERROR
        "Unsupported platform: ${POCO_PLATFORM}. Check platforms folder for "
        "possibilities"
    )
endif()
