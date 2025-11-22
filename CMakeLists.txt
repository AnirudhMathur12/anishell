cmake_minimum_required(VERSION 3.10)

project(SimpleShell C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

add_compile_options(-Wall -Wextra -pedantic)
add_compile_options(-fsanitize=address -g)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

add_executable(anishell
                    main.c
                    parser.c
                    input.c
                    autocomplete.c
                    shell.h)
