# Unix Shell
This project implements a simple Unix shell. The shell is the heart of the commandline interface, and thus is central to the Unix/C programming environment. Mastering use of the shell is necessary to become proficient in this world; knowing how the shell itself is built is the focus of this project.


# Overview
The shell should operate in this basic way: when you type in a command (in response to its prompt), the shell creates a child process that executes the command you entered and then prompts for more user input when it has finished.

# Program Specifications
## Basic Shell: `dash`
Your basic shell, called **dash**, is basically an interactive loop: it repeatedly prints a prompt `dash>`, parses the input, executes the command specified on that line of input, and waits for the command to finish. This is repeated until the user types `exit`.

The shell can be invoked with either no arguments or a single argument; anything else is an error. Here is the no-argument way:
```
prompt> ./dash
dash>
```

At this point, dash is running, and ready to accept commands. Type away!
The mode above is called interactive mode. It allows the user to type commands directly. The shell also supports a batch mode, which instead reads input from a batch file and executes commands from therein. Here is how you run the shell with a batch file named `batch.txt`:
```
prompt> ./dash batch.txt
```

The shell is structured such that it creates a process for each new command. The shell is be able to parse a command and run the program corresponding to the command. For example, if the user types `ls -la /tmp`, the shell will run the program `/bin/ls` with the given arguments `-la` and `/tmp`.

# Structure
## Basic Shell
The shell is very simple (conceptually): it runs in a while loop, repeatedly asking for input to tell it what command to execute. It then executes that command. The loop continues indefinitely, until the user enters the built-in command exit, at which point it exits.

## Paths
It turns out that the user must specify a path variable to describe the set of directories to search for executables; the set of directories that comprise the path are sometimes called the search path of the shell. The path variable contains the list of all directories to search, in order, when the user types a command.

## Built-in Commands
Whenever the shell accepts a command, it will check whether the command is a built-in command or not. If it is, it will not be executed like other programs. Instead, the shell will invoke its own implementation of the built-in command. For example, to implement the `exit` built-in command, you simply call `exit(0)`; in the dash source code, which then will exit the shell.

In this shell, `exit`, `cd`, and `path` have been implemented as built-in commands.

## Redirection
Many times, a shell user prefers to send the output of a program to a file rather than to the screen. Usually, a shell provides this nice feature with the `>` character. Formally this is named as redirection of standard output. This shell also include this feature, but with a slight twist.

For example, if a user types, `ls -la /tmp > output`, nothing is printed on the screen. Instead, the standard output of the `ls` program is rerouted to the file `output`. In addition, the standard error output of the file is also rerouted to the file `output`.

## Parallel Commands
The shell will also allow the user to launch parallel commands. This is accomplished with the `&` operator as follows.
```
dash> cmd1 & cmd2 args1 args2 & cmd3 args1
```
In this case, instead of running `cmd1` and then waiting for it to finish, your shell runs `cmd1`, `cmd2`, and `cmd3` (each with whatever arguments the user has passed to it) in parallel, before waiting for any of them to complete.

## Program Errors
Shell prints this one and only error message whenever it encounters an error of any type:
```
char error_message[30] = "An error has occurred\n";
write(STDERR_FILENO, error_message, strlen(error_message));
```

# THAT'S ALL
