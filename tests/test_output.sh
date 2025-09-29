#!/bin/sh
# Script used to capture the output of the command
#
# SPDX-FileCopyrightText: Copyright contributors to the poco project.
# SPDX-License-Identifier: MIT

command=$1
output_file=$2

$command 1> $output_file 2> /dev/null
