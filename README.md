# Anishell
A lightweight shell written in C, as a hobby-project
> **Note**
> The shell only runs on POSIX compliant operating systems. 

## Table of Contents
1. [Features](#features)
2. [Getting Started](#getting-started)
3. [Usage](#usage)
4. [Project Structure](#project-structure)
5. [Building & Dependencies](#building--dependencies)
6. [Contributing](#contributing)
7. [License](#license)

## Features
  - Interactive cmd prompt; reads user input and parses commands
  - Support for lightweight configuration via `config.c`
  - Built-in autocomplete using tabs and UP/DOWN arrow keys via `autocomplete.c`
  - Simple parser via `parser.c`
  - No external dependencies: raw-mode terminal handling implemented manually.
## Getting Started
1. Clone the repository
  ```sh
   git clone https://github.com/AnirudhMathur12/anishell.git  
   cd anishell
```
2. Create a build directory, run CMake, compile and run the executable:
  ```sh
   mkdir build && cd build  
   cmake ..  
   make
   ./anishell
```
Or simply run the build.sh file
  
```sh
  ./build.sh
```

## Usage
- Launch the shell: you’re presented with a prompt.
- On startup the shell will run `~/.anishellrc`.
- Enter commands: the shell will parse and execute (via standard exec/fork) depending on the commands implementation.
- Use tab/autocomplete support while typing.
- Use commands as you would in a typical Unix‐like shell (cd, ls, etc), subject to what’s implemented.
## Project Structure

```
├── autocomplete.c     # autocomplete logic  
├── config.c           # configuration parameters  
├── input.c            # reading input in raw mode, handling line editing  
├── parser.c           # parsing command line into tokens/args  
├── shell.h            # common definitions/interfaces  
├── main.c             # entry point, shell loop  
├── CMakeLists.txt     # build config
├── .gitignore  
└── LICENSE            # Apache-2.0 License  
```
## Building & Dependencies
 - Requires a C compiler (e.g., gcc or clang) supporting C99 or newer.
 - Uses CMake (version ≥ 3.x) for build configuration.
 - Platform: Developed and tested on Unix-like systems (Linux/macOS).
 - No additional external libraries beyond standard C library.
## Contributing
 - Contributions are welcome! Here’s how you can help:
 - Report issues or feature requests via the Issues tab.
 - Fork the repo, make your changes, and submit a pull request (PR).
 - Suggested enhancements:
   - Add more built‐in commands.
   - Improve line‐editing features (history, arrow keys).
   - Job control
   - Conditional operators (`&&` and `||`)
 - Please follow the coding style (e.g., indentation, naming) used in the project.
## License
This project is licensed under the Apache License 2.0.
See the LICENSE file for full details.

Last updated: 23/11/2025
