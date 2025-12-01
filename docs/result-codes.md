<!-- 
SPDX-FileCopyrightText: Copyright contributors to the poco project.
SPDX-License-Identifier: MIT
-->

# Result Codes

Allows a unified result code separate from the standard codes, (EIO, etc.) which is
POSIX.

All poco result codes are encoded within a 32-bit error space. By default, we
reserve the error space 0x00000000 to 0x0000FFFF for library use. Other applications
can either define their own groupins with the remaining values if needed.

Within the poco library error space, it is split between 0xXXYY, where XX
denotes the module and YY denotes the error code.

This file defines the numbers assigned to each module. The specific error codes
will be defined within the module itself (except the general error space).
