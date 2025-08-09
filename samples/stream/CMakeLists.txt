# SPDX-FileCopyrightText: Copyright contributors to the poco project.
# SPDX-License-Identifier: MIT

add_executable(sample_stream sample_stream.c)
target_link_libraries(sample_stream PRIVATE poco::poco)