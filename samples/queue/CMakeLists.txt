# SPDX-FileCopyrightText: Copyright contributors to the poco project.
# SPDX-License-Identifier: MIT

add_executable(sample_queue sample_queue.c)
target_link_libraries(sample_queue PRIVATE poco::poco)