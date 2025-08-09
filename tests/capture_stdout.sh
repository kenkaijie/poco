#!/bin/sh
# This script is used to capture the output of a command and save it to a file.
# Used to allow ctest to check the output.
#
# SPDX-FileCopyrightText: Copyright contributors to the poco project.
# SPDX-License-Identifier: MIT

# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <command> <output_file>"
    exit 1
fi
# Assign arguments to variables
command=$1
output_file=$2
# Execute the command and capture the output

$command 1> $output_file 2> /dev/null

cat $output_file
