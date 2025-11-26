cmake_minimum_required(VERSION 3.10)

# apple clang is nerfed (no LSan), use brew llvm if available
if(EXISTS "/opt/homebrew/opt/llvm/bin/clang")
    message(STATUS "found brew llvm, switching compilers")
    set(CMAKE_C_COMPILER "/opt/homebrew/opt/llvm/bin/clang")
else()
    message(WARNING "using system clang (leaks might be ignored)")
endif()

project(anishell C)

# make neovim happy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

if(DEBUG)
    message(STATUS "debug mode on")

    add_compile_options(
        -g                           # symbols
        -O1                          # readable stack traces
        -fsanitize=address,undefined # catch leaks + logic errors
        -fno-omit-frame-pointer
        -Wall -Wextra -pedantic      # basic warnings
    )

    add_link_options(-fsanitize=address,undefined)
endif()

add_executable(anishell
    main.c
    parser.c
    input.c
    autocomplete.c
    config.c
    shell.h
)
