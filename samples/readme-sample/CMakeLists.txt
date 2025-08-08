# SPDX-FileCopyrightText: Copyright contributors to the poco project.
# SPDX-License-Identifier: MIT

add_executable(readme_sample main.c)
target_link_libraries(readme_sample PRIVATE poco::poco)
