# mini-shell 

mini-shell is a C program that interprets and performs Unix commands. 
This program is built and tested to run on top of the Bash Unix Shell (bash). 

## Compilation Instructions
NOTE: A GCC / C Compiler must be installed to compile this program.

In your terminal navigate to the directory containing minish.c

Linux:<br/>
To compile the program enter:
```
gcc --std=gnu99 -o minish minish.c
```
To run the program execute: 
```
./minish
```

Windows:<br/>
Note: There is an issue when compiling using MinGW or MSYS compiler. These compilers use the interfaces natively provided by Windows, 
and because Windows does not 100% abide to POSIX standards there is no <sys/wait.> header file available and a corresponding error will be
thrown upon compilation. I would suggest either installing an emulator like Cygwin and compiling in the Cygwin terminal or setting up Windows Subsystem for Linux (WSL) on your system and compile within a WSL shell.

MacOS \ OS X:<br/>
I do not currently use MacOS. Hence, I have not compiled in this OS. However, MacOS is based on Unix and follows POSIX standards, so as long as a GCC compiler is installed on a MacOS system the Linux compilation instruction is more or less the same for MacOS. 

## Shell Instructions

The program runs a shell where the colon ```:``` symbol prompts commands.
The supported syntax for a command is: 
```
{command} {arg1 arg2 ...} {< input_file} {> output_file} {&}
```
All words in the command line are separated by spaces. Unix commands can be executed in the shell.

Foreground and background commands are supported. ```&``` at the end of a command indicates that it is
designated to be run as background process. 

The program also has three built-in commands:
1. ```exit``` exits out of the shell.
2. ```status``` outputs the exit status or terminating signal of last the foreground process executed on the shell. Takes no arguments.
3. ```cd``` changes the shell's working directory. With no arguments, changes working directory to HOME directory. With
	arguments, takes a relative or absolute path to a new working directory.

