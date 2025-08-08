<!-- 
SPDX-FileCopyrightText: Copyright contributors to the poco project.
SPDX-License-Identifier: MIT
-->
# Scheduler Choice

poco comes with 2 basic schedulers; round-robin and priority. The user is free to choose
one to use within their projects depending on the application or create their own.

A general summary of the tradeoffs between the 2 types is found below.

Round-Robin

- Prevents resource starvation.
- Lower responsiveness.

Priority

- Higher responsiveness.
- Susceptible to resource starvation (if high priority coroutine never blocks)
- Introduces a class of bugs such as priority inversion.
