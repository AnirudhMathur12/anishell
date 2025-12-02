cmake_minimum_required(VERSION 3.10)

# only mess with the compiler path if we are on macos
# linux users usually have a working sanitize setup by default
if(APPLE)
    if(EXISTS "/opt/homebrew/opt/llvm/bin/clang")
        message(STATUS "macos detected: switching to brew llvm for leaks")
        set(CMAKE_C_COMPILER "/opt/homebrew/opt/llvm/bin/clang")
    else()
        message(WARNING "macos detected but brew llvm missing. leaks might be ignored.")
    endif()
endif()

project(SimpleShell C)

# make neovim happy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

if(DEBUG)
    message(STATUS "debug mode on")

    # linux/gcc might need explicit libasan linking sometimes, but clang usually handles it
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
    utils.c
    shell.h
)
