# Simple Shell
 

## Table of Contents

- [Overview](#assignment-overview)
- [Features](#features)
- [Useful Information](#useful-information)

##  Overview

- The shell prompts the user for a command.
- The command and its arguments are separated. For example, in `cat prog.c`, `cat` is the command and `prog.c` is its argument.
- The command is executed in a child process, created using the `fork()` system call.
- Command execution is handled using the `exec()` family of functions.
- You can run commands in the background using `&`.

## Features

### Basic Shell

- Show a prompt and take user input.
- Handle commands, arguments, and background execution.

### Built-in Commands

- `echo`: Print its arguments.
- `cd`: Change directory or print current directory.
- `pwd`: Print the present working directory.
- `exit`: Terminate the shell.
- `fg`: Bring a background job to the foreground.
- `jobs`: List all background jobs.

### Advanced Features

- **Output Redirection**: Redirect the output of a command to a file. (e.g., `ls > out.txt`)
- **Command Piping**: Implement command piping using pipes. (e.g., `ls | wc -l`)


## Useful Information

- `fork()`: Create a new process.
- `execvp()`: Execute a program in a process.
- `exit()`: Terminate a process.
- `pipe()`: Create a communication channel between processes.
- `dup()`: Duplicate a file descriptor.

Copyright ~ Chris Hatoum 
