# SPDX-FileCopyrightText: Copyright contributors to the poco project.
# SPDX-License-Identifier: MIT

add_executable(sample_mutex sample_mutex.c)
target_link_libraries(sample_mutex PRIVATE poco::poco)
