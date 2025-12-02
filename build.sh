#!/usr/bin/env zsh

BUILD_DIR="build"

if [[ ! -d "$BUILD_DIR" ]]; then
    echo "Creating a build directory..."
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

cmake .. && make

if [[ $status -eq 0 ]]; then
    ./anishell
fi
