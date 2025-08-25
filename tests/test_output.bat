:: Script used to capture the output of the command
::
:: SPDX-FileCopyrightText: Copyright contributors to the poco project.
:: SPDX-License-Identifier: MIT

set command=%1
set output_file=%2

%command% > %output_file%
