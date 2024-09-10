# EE 468 Project: Simple Shell with Pipes

## Overview

This project involves modifying a simple shell program to support command pipelining using pipes (`|`). The initial version of the shell, provided in the `sshell.c` file, does not handle redirection or pipes. Your task is to enhance this shell to handle pipes, enabling it to execute a sequence of commands where the output of one command becomes the input of the next.

### Project Files

- **sshell.c**: The initial simple shell program that needs modification.
- **pipe.c**: An example implementation of pipes using `dup2`.
- **EE367Lab3**: A lab document from EE 367L that involves pipes and the `exec` command.
- **EE367LOverviewClientServer**: Overview of client-server processes.
- **UNIX-Intro**: Introduction to UNIX (or Linux) commands.
- **almamater**: A file for testing the shell.

### Task

Modify the `sshell.c` program to implement the following functionalities:

1. **Parse Input Line**: Parse the input command line to identify the pipe symbols (`|`).
2. **Process Creation**: Use `fork()` and `execvp()` to launch each command as a separate process.
3. **Pipe Connection**: Connect processes using pipes, making sure that the output of one process is used as the input for the next.
4. **Pipe Handling**: The shell should handle up to 9 pipes for full credit. For partial credit, handling up to 1 pipe is required.

### Implementation Details

- **Parsing Input**: Modify the shell to split the input command line into separate commands based on the pipe symbol (`|`).
- **Pipe Creation**: Implement pipe creation and management. Refer to `pipe.c` for guidance on using `dup2` to redirect input and output.
- **Process Management**: Ensure that each command runs in its own process using `fork()` and `execvp()`.
- **Avoid Zombie Processes**: Ensure that processes are properly waited for and terminated to avoid zombie processes.

### Example Usage

Given an input like `cat <file name> | grep and | hexdump -x`, the shell should:

1. Execute `cat <file name>`, where `<file name>` is the name of the file.
2. Pipe the output of `cat` to `grep and`.
3. Pipe the output of `grep and` to `hexdump -x`.

### Testing

To test the shell, you can use the following command:

```bash
cat almamater | grep and | grep her | hexdump -
