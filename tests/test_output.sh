#!/bin/sh
# This script is used to capture the output of a command and save it to a file and to
# compare that file with an expected output file.
#
# SPDX-FileCopyrightText: Copyright contributors to the poco project.
# SPDX-License-Identifier: MIT

# Check if the correct number of arguments is provided
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <command> <output_file> <expected_output>"
    exit 1
fi
# Assign arguments to variables
command=$1
output_file=$2
expected_output=$3
# Execute the command and capture the output

$command 1> $output_file 2> /dev/null

diff $output_file $expected_output
