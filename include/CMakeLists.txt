# SPDX-FileCopyrightText: Copyright contributors to the poco project.
# SPDX-License-Identifier: MIT

target_sources(
    poco
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES 
            ${CMAKE_CURRENT_SOURCE_DIR}/poco/schedulers/round_robin.h
            ${CMAKE_CURRENT_SOURCE_DIR}/poco/context.h
            ${CMAKE_CURRENT_SOURCE_DIR}/poco/coro.h
            ${CMAKE_CURRENT_SOURCE_DIR}/poco/coro_raw.h
            ${CMAKE_CURRENT_SOURCE_DIR}/poco/event.h
            ${CMAKE_CURRENT_SOURCE_DIR}/poco/intracoro.h
            ${CMAKE_CURRENT_SOURCE_DIR}/poco/mutex.h
            ${CMAKE_CURRENT_SOURCE_DIR}/poco/poco.h
            ${CMAKE_CURRENT_SOURCE_DIR}/poco/queue.h
            ${CMAKE_CURRENT_SOURCE_DIR}/poco/queue_raw.h
            ${CMAKE_CURRENT_SOURCE_DIR}/poco/result.h
            ${CMAKE_CURRENT_SOURCE_DIR}/poco/scheduler.h
            ${CMAKE_CURRENT_SOURCE_DIR}/poco/semaphore.h
            ${CMAKE_CURRENT_SOURCE_DIR}/poco/stream.h
)
