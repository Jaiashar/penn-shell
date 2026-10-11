Names: Mytreyi Narayanaswamy, Jai Ashar

PennKey: mytreyi, jaiashar

## List of submitted files
1. penn_shell.c
2. interrupt_handlers.h
3. interrupt_handlers.c
4. error_handlers.h
5. error_handlers.c
6. linked_list.c
7. linked_list.h
8. input_reader.c
9. input_reader.h

## Overview 
PennShell is a custom shell that restricts the runtime of processes that are executed. The user can redirect the input and output, as well as pipe multiple system calls. Terminal control and process group isolation were implemented. Builtin commands jobs, bg and fg are provided for users to restart stopped processes, or bring background processes to the foreground. To exit the shell, the user must type Ctrl-D into an empty prompt.

## Code layout and description

1. penn_shell.c - this is the main file that displays the shell and forks child processes
2. interrupt_handlers.h - defines interrupt handlers
3. interrupt_handlers.c - implements the defined interrupt handlers 
4. error_handlers.h - defines error handling methods 
5. error_handlers.c - implements methods in error_handlers.h
6. linked_list.h - defines linked list and methods
7. linked_list.c - implements linked list for background job queue
8. input_reader.h - defines methods to read input from command line / file
9. input_reader.c - defintion of methods in inout_reader.h

## General comments
The shell handles errors that could occur when various system calls are made, and the code was evaluated to ensure that no memory leaks occur. Both team members contributed roughly equally, and coding and discussions were done over shared screen sessions on Zoom.