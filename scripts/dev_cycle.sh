#!/bin/bash
# Runs a development cycle that enables fast iteration.

find ./ | entr -c -s 'scripts/refresh_compile_commands.sh && bazel test ...'
