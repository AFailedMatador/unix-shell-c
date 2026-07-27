# Unix Shell in C

*Built for CMSC 14400 (Systems Programming II), University of Chicago.*

A command-line shell implemented from scratch in C, supporting both interactive use and batch script execution.

## Features

- **Command execution** — parses and runs external commands via `fork`/`execvp`/`waitpid`
- **Built-ins** — `cd` and other built-in commands handled without spawning a subprocess
- **Output redirection** — single (`>`) and chained/advanced multi-command redirection, with validation of malformed or illegal redirection syntax
- **Batch mode** — runs a script of commands from a file, matching output against expected results
- **Error handling** — bad commands, empty input, and invalid arguments are caught and reported without crashing the shell

## Build & run

```bash
make
./myshell                # interactive mode
./myshell batch-files/gooduser_basic   # run a batch script
```

## Testing

`batch-files/` contains 16 test scripts covering built-ins, argument parsing, multi-command pipelines, and both simple and advanced redirection (including deliberately malformed input). Expected stdout/stderr for each is checked into `expected-output/` for diffing against actual shell output.
