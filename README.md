# MiniShell 

MiniShell is a C program that interprets and performs Unix commands. 
This program was designed to run on the Bash Unix Shell (bash). 

## Instructions
NOTE: A GCC/C Compiler must be installed to compile and run this program.

Using bash navigate to the directory containing minish.c
To compile the program, enter 'gcc --std=gnu99 -o minish minish.c' into Bash.
To run the program, enter './smallsh'

The program runs a shell where the colon [:] symbol prompts command lines.
The supported syntax for a command line is: 'command [arg1 arg2 ...] [< input_file] [> output_file] [&]'.
All words in the command line are separated by spaces. Unix commands can be run on the shell, but the above
syntax must be used. 

Foreground and background commands are supported for Unix commands. [&] at the end of a command line indicates that it is a
designated background command. 

The program also has three built-in commands:
exit : exits out of the shell.
status : outputs the exit status or terminating signal of last the foreground process executed on the shell. Takes no arguments.
cd : changes the shell's working directory. With no arguments, changes working directory to HOME directory. With
	arguments, takes a relative or absolute path to a new working directory.

