#!/bin/bash
# Prints the top commit hash, to be embedded as a version in the binary.

echo STABLE_GIT_COMMIT $(git rev-parse HEAD)
