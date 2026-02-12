# SPDX-FileCopyrightText: Copyright contributors to the poco project.
# SPDX-License-Identifier: MIT

add_executable(sample_custom_primitive sample_custom_primitive.c observable_u32.c)
target_link_libraries(sample_custom_primitive PRIVATE poco::poco)
