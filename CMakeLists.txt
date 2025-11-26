cmake_minimum_required(VERSION 3.10)

project(SimpleShell C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

if(EXISTS "/opt/homebrew/opt/llvm/bin/clang")
    message(STATUS "Found Homebrew LLVM: Switching compiler to enable LeakSanitizer")
    set(CMAKE_C_COMPILER "/opt/homebrew/opt/llvm/bin/clang")
else()
    message(WARNING "Homebrew LLVM not found. Using system Clang (Leak detection might fail).")
endif()

if(DEBUG)
    add_compile_options(-g -O1 -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls)
    add_link_options(-fsanitize=address)
    message(STATUS "Debug Mode: ON (AddressSanitizer + LeakSanitizer enabled)")
endif()

if(ASAN)
    message("Annoying mode is on")
    add_compile_options(-Wall -Wextra -pedantic)
    add_compile_options(-fsanitize=address)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

add_executable(anishell
                    main.c
                    parser.c
                    input.c
                    autocomplete.c
                    config.c
                    shell.h)
