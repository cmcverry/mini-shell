/*Author: Christian McVerry
Last Updated: 07/15/2022
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAXCHARLENGTH 2048
#define MAXARGS 512

// global variables 
bool blockBackground = false;
bool atPrompt = true;
pid_t foregroundPid;

// defines struct command 
// used for parsing user input commands
struct command
{
    char cmd[255];
    char* args[MAXARGS];
    char inputFile[255];
    char outputFile[255];
    bool background;
};


/*
* Function: parse
* 
* Receives: char* string
* Initializes command struct parsedInput with all args array elements set to NULL and background set to false.
* Uses to strtok and " " space delimiter to parse through string (user input command). First token is set
* as cmd in parsedInput and subsequent space delimited words are added to the args array. Loops through until token
* is NULL. Also, Checks for <, >, and & characters, taking the subsequent tokens after < and or > as input and output 
* file names, and setting parsedInput's background data member to true if a sole & character is found. 
* Returns: pointer to command struct parsedInput
* 
*/
struct command* parse(char* string)
{
    // initializes command struct parsedInput of size command struct
    struct command* parsedInput = malloc(sizeof(struct command));
    // sets all elements in args array to NULL
    for (int i = 0; i < MAXARGS; i++)
    {
        parsedInput->args[i] = NULL;
    }
    parsedInput->background = false;

    char delimiter[2] = " ";
    // first space delimited word in user input is the command 
    char* token = strtok(string, delimiter);
    strcpy(parsedInput->cmd, token);

    // i used as incrementing array index 
    int i = 0;

    //while token is not NULL
    while (token)
    {
        // if token is <, subsequent token is input file
        if (!strcmp(token, "<"))
        {
            token = strtok(NULL, delimiter);
            strcpy(parsedInput->inputFile, token);
        }
        // if token is >, subsequent token is output file
        else if (!strcmp(token, ">"))
        {
            token = strtok(NULL, delimiter);
            strcpy(parsedInput->outputFile, token);
        }
        // if token is &, parsedInput background data member set to true 
        else if (!strcmp(token, "&"))
        {
            parsedInput->background = true;
        }
        // else token is a command argument and added to the args array
        else
        {
            parsedInput->args[i] = strdup(token);
            i++;
        }
        token = strtok(NULL, delimiter);
    }
    return parsedInput;
}


/*Function unused in shellsh program; used for debugging parsed input*/
void printParsedInput(struct command* input)
{
    printf("cmd: %s |", input->cmd);
    int i = 0;
    while (input->args[i] != NULL)
    {
        printf(" arg: %s ", input->args[i]);
        i++;
    }
    printf("| input: %s |", input->inputFile);
    printf(" output: %s |", input->outputFile);
    printf(" background: %s \n", input->background ? "true" : "false");
    fflush(stdout);
}


/*
* Function: execOther
* 
* Receives: struct command* input, int* exitStatus, struct sigaction actionOnSigInt, struct sigaction actionOnSIGTSTP
* Creates a new child process. Checks if command is meant to be performed on a background process.
* In the child process sets up input and output redirection based on user command and calls execvp() 
* with the user's command and arguments. Using waitpid(), if foreground process waits for process to finish, if background process
* calls waitpid() with options parameter set to WNOHANG so that the parent process does not wait for background process to finish. 
* Returns: nothing
* 
*/
void execOther(struct command* input, int* exitStatus, struct sigaction actionOnSIGINT, struct sigaction actionOnSIGTSTP)
{
    // creates new child process
    pid_t spawnPid = fork();

    // switch statement evaluates value returned to spawnPid from fork()
    switch (spawnPid)
    {
    
    // -1 value indicates fork() failure
    case -1:
        perror("fork() failed.");
        exit(1);
        break;

    // 0 value indicates succesful creation of child process 
    case 0:
        // inside child process
        // child process ignores SIGTSTP
        actionOnSIGTSTP.sa_handler = SIG_IGN;
        sigaction(SIGTSTP, &actionOnSIGTSTP, NULL);

        // Checks based on command input if background process and if foreground-only mode is off
        if (input->background && !blockBackground)
        {
            // if so, sets input/output redirection based on command input
            // if no input file is specified, file descriptor directs to /dev/null
            if (!strcmp(input->inputFile, ""))
            {
                int inputFD = open("/dev/null", O_RDONLY);
                // -1 return value indicates error
                if (inputFD == -1)
                {
                    perror("inputFile open(/dev/null) error");
                    exit(1);
                }
                // redurects stdin to file descriptor /dev/null
                int result = dup2(inputFD, 0);
                if (result == -1)
                {
                    perror("input dup2() error");
                    exit(1);
                }
            }
            else
            {
                // if input file is specified, file descriptor directs to file
                int inputFD = open(input->inputFile, O_RDONLY);
                if (inputFD == -1)
                {
                    // -1 return value indicates failure to open input file
                    printf("cannot open %s for input\n", input->inputFile);
                    fflush(stdout);
                    exit(1);
                }
                // redurects stdin to file descriptor specified input file
                int result = dup2(inputFD, 0);
                if (result == -1)
                {
                    perror("input dup2() error");
                    exit(1);
                }
            }
            // if no output file is specified, file descriptor directs to /dev/null
            if (!strcmp(input->outputFile, ""))
            { 
                int outputFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (outputFD == -1)
                {
                    perror("outputFile open(/dev/null) error");
                    exit(1);
                }

                int result = dup2(outputFD, 1);
                if (result == -1)
                {
                    perror("output dup2() error");
                    exit(1);
                }
            }
            else
            {
                // if output file is specified, file descriptor directs to file
                int outputFD = open(input->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (outputFD == -1)
                {
                    printf("cannot open %s for output\n", input->outputFile);
                    fflush(stdout);
                    exit(1);
                }

                int result = dup2(outputFD, 1);
                if (result == -1)
                {
                    perror("output dup2() error");
                    exit(1);
                }
            }
        }
        // else either foreground-mode only is on or command input does not specify background process
        else
        {
            // SIGINT handler set to default behavior for foreground child processes 
            actionOnSIGINT.sa_handler = SIG_DFL;
            sigaction(SIGINT, &actionOnSIGINT, NULL);
            // updates foregroundPid with most process id of most recent foreground child process
            foregroundPid = getpid();

            if (strcmp(input->inputFile, ""))
            {
                int inputFD = open(input->inputFile, O_RDONLY);
                if (inputFD == -1)
                {
                    printf("cannot open %s for input\n", input->inputFile);
                    fflush(stdout);
                    exit(1);
                }

                int result = dup2(inputFD, 0);
                if (result == -1)
                {
                    perror("input dup2() error");
                    exit(1);
                }
            }

            if (strcmp(input->outputFile, ""))
            {
                int outputFD = open(input->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (outputFD == -1)
                {
                    printf("cannot open %s for output", input->outputFile);
                    fflush(stdout);
                    exit(1);
                }

                int result = dup2(outputFD, 1);
                if (result == -1)
                {
                    perror("output dup2() error");
                    exit(1);
                }
            }
        }

        // calls execvp with user input command and array of arguments
        int executeProgram = execvp(input->cmd, input->args);
        if (executeProgram == -1)
        {
            // if execvp returns -1 to executeProgram, then file/directory could not be found
            printf("%s: no such file or directory\n", input->cmd);
            fflush(stdout);
            exit(1);
        }
        break;
    
    default:
        // inside parent process
        // if background process was not specified in user input or foreground-only mode is on
        if (!input->background || blockBackground)
        {
            // parent process waits for foreground child process to end
            pid_t somePid = waitpid(spawnPid, exitStatus, 0);
            // if foreground child process was terminated by a signal (SIGINT via ctrl-c command)
            // message states terminating signal 
            if (WIFSIGNALED(*exitStatus))
            {
                printf("terminated by signal %d\n", WTERMSIG(*exitStatus));
                fflush(stdout);
            }
            break;
        }
        // else is background child process and foreground-only mode is off 
        else
        {
            // parent process requests background child process status
            // immediately returns if child process has not exited; 
            // parent process does not wait for child process to end
            pid_t somePid = waitpid(spawnPid, exitStatus, WNOHANG);
            printf("background pid is %d\n", spawnPid);
            fflush(stdout);
            break;
        }

    }
}


/*
* Function: variableExpansion
* 
* Receives: char* str, char* pid
* Initializes a new string. Iterates by character through the string argument, copying character by
* character to the new string, searching for '$$', replacing '$$' with the parent process id string 
* argument in the new string. 
* Returns pointer to the new string 
* 
* Resource: https://stackoverflow.com/questions/27160073/replacing-words-inside-string-in-c
*/
char* variableExpansion(char* str, char* pid)
{
    char target[3] = "$$";
    int targetLength = strlen(target);
    int replaceLength = strlen(pid);

    // defines string newStr with a safe abundance of allocated memory
    char* newStr = malloc(strlen(str) * replaceLength + 1);
    // pointer to newStr
    char* newStrPtr = newStr;

    // while *str does not point to a null character
    while (*str)
    {
        // compares the next two character in the string with "$$"
        if (!strncmp(str, target, targetLength))
        {
            // if matched, process id is concatenated to current position of newStr pointer
            strcat(newStrPtr, pid);
            // performs pointer simple arithmetic to move to next characters in strings 
            str += targetLength;
            // newStr pointer effectively moves to end of non-null characters in newStr string
            newStrPtr += replaceLength;
        }
        else
        {
            // copy character and increment pointers 
            *newStrPtr = *str;
            newStrPtr++;
            str++;
        }
    }
    return newStr;
}


/*
* Function: handle_SIGTSTP
* 
* Receives: int signo
* Custom handler for SIGTSTP. Effectively toggles blockBackground true/false.
* Prints a corresponding message informing user of state of foreground-only mode.
* Waits for foreground process to terminate, before printing message.
* Returns nothing
*/
void handle_SIGTSTP(int signo)
{

    if (!blockBackground)
    {
        blockBackground = true;
        //waits for foreground process to terminate, if it has not already
        pid_t somePid = waitpid(foregroundPid, NULL, 0);
        //checks if ctrl-z command was entered at prompt or during child foreground process
        if (atPrompt)
        {
            char* message = "\nEntering foreground-only mode (& is now ignored)\n: ";
            write(STDOUT_FILENO, message, 53);
        }
        else
        {
            char* message = "\nEntering foreground-only mode (& is now ignored)\n";
            write(STDOUT_FILENO, message, 51);
        }
        fflush(stdout);
    }
    else
    {
        blockBackground = false;
        //waits for foreground process to terminate, if it has not already
        pid_t somePid = waitpid(foregroundPid, NULL, 0);
        //checks if ctrl-z command was entered at prompt or during child foreground process
        if (atPrompt)
        {
            char* message = "\nExiting foreground-only mode\n: ";
            write(STDOUT_FILENO, message, 32);
        }
        else
        {
            char* message = "\nExiting foreground-only mode\n";
            write(STDOUT_FILENO, message, 30);
        }
        fflush(stdout);
        
    }
}


/*
* Function: runSH
* 
* Receives: nothing
* Initializes variables and sigaction structs; loop prompts users for commands
* until 'exit' command is entered. Supports three built-in commands: cd, status, and exit
* Supports unix commands. While loop checks for any finished background zombie-child processes. 
* 'exit' command  effectively exits the shell.
* Returns: nothing
*/
void runSH()
{
    char input[MAXCHARLENGTH];
    char* end = "exit\n";
    int exitStatus = 0;
    // gets process id of shell
    pid_t shellProcessId = getpid();
    char pidStr[7];
    // converts shell process id to string
    sprintf(pidStr, "%d", shellProcessId);


    // initializes sigaction struct SIGINT_action
    struct sigaction SIGINT_action = { 0 };
    SIGINT_action.sa_handler = SIG_IGN;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT_action, NULL);

    // initializes sigaction struct SIGTSTP_action with custom handler 
    // handle_SIGTSTP and SA_RESTART flag
    struct sigaction SIGTSTP_action = { 0 };
    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);


    printf(": ");
    fflush(stdout);
    // gets user command/input
    fgets(input, MAXCHARLENGTH, stdin);

    while (strcmp(input, end))
    {
        //sigaction(SIGTSTP, &SIGTSTP_action, NULL);

        if ((!strncmp(input, "#", 1)) || (!strcmp(input, "\n")))
        {
            // Do nothing
        }
        else 
        {
            // removes trailing '\n' from input
            char* pos;
            if ((pos = strchr(input, '\n')) != NULL)
                *pos = '\0';

            // performs '$$' expansion and parses input into command struct's data members 
            char* expandedInput = variableExpansion(input, pidStr);
            struct command* parsedInput = parse(expandedInput);
    

            // executes built-in status command
            if (!strcmp(parsedInput->cmd, "status"))
            {
                // checks exitStatus of last foreground process 
                // prints message about exit status or whether process exited
                // because of signal termination 
                if (WIFEXITED(exitStatus))
                {
                    printf("exit value %d\n", WEXITSTATUS(exitStatus));
                    fflush(stdout);
                }
                else
                {
                    printf("terminated by signal %d\n", WTERMSIG(exitStatus));
                    fflush(stdout);
                }
            }

            // executes built-in cd command
            else if (!strcmp(parsedInput->cmd, "cd"))
            {
                // checks first element in arguments array for directory path
                if (parsedInput->args[1])
                {
                    chdir(parsedInput->args[1]);
                }
                else 
                {
                    chdir(getenv("HOME"));
                }
            }

            // executes non-built-in commands
           else 
           {
                execOther(parsedInput, &exitStatus, SIGINT_action, SIGTSTP_action);
           }
        }

        // watches for and reaps any completed/terminated background child processes
        pid_t spawnPid;
        while ((spawnPid = waitpid(-1, &exitStatus, WNOHANG)) > 0)
        {
            printf("background pid %d is done: ", spawnPid);
            if (WIFEXITED(exitStatus))
            {
                printf("exit value %d\n", WEXITSTATUS(exitStatus));
                fflush(stdout);
            }
            else
            {
                printf("terminated by signal %d\n", WTERMSIG(exitStatus));
                fflush(stdout);
            }
        }

        printf(": ");
        fflush(stdout);
        atPrompt = true;
        fgets(input, MAXCHARLENGTH, stdin);
        atPrompt = false;
    }
}


int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        printf("Program is taking too many arguments\n");
        return EXIT_FAILURE;
    }
    runSH();

    return EXIT_SUCCESS;
}