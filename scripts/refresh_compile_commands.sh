#!/bin/bash
# Refreshes compile_commands.json.

bear -- bazel test //...
