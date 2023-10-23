// Include all necessary libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// System command path
char initialPath[100] = "/bin /usr/bin";

void CheckRedirection(char **redirection, char **cmd, char **filename, int* isRedirection);
void Execute_CMD(char **input_cmds);
void Execute_Redir_CMD(char **input_cmds, char *filename);
void ParseInput(char **input_cmds, char *input,char *delimiter);
void DisplayError();
int  handle_builtin_commands(char **input_cmds);
void ParseCommand(char *commands);

// Main function
int main(int argc, const char * argv[]){
    
    // if more than two arguments, return error
    if(argc > 2){
        DisplayError();
        exit(0);
    }
    // BATCH MODE
    if(argc == 2){
        // open the file to read command lines from
        FILE *input_file = fopen(argv[1], "r");
        if(input_file == NULL){
            DisplayError();
            exit(0);
        }
        char *input_line = NULL;
        size_t input_line_size = 0;

        // read and execute each command line
        while(getline(&input_line, &input_line_size, input_file)!=-1){
            ParseCommand(input_line);
    	}
        fclose(input_file);
        exit(0);
    }
    // INTERACTIVE MODE
    while(1){
        printf("dash> ");

        // get inputted command line
        char *input = NULL;
        size_t input_size = 0;
        getline(&input, &input_size, stdin);

        // execute command line
        ParseCommand(input);
    }
    return 0;
}

// function to tokenize a given command based on a given delimiter
void ParseInput(char **input_cmds, char *input,char *delimiter){
    char *cmd;

    int i = 0;
    cmd = strtok(input, delimiter);

    // store each token in the input_cmds array
    while(cmd != NULL){
        input_cmds[i] = cmd;
        i++;
        cmd = strtok(NULL,delimiter);
    }
    input_cmds[i] = NULL;
}

// function to implement the general error message
void DisplayError(){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

// function to check if a given command is a redirection command or not
void CheckRedirection(char **redirection, char **cmd, char **filename, int* isRedirection){
    // this is to parse commands like 'ls F>t.txt' where no spaces are there
    int i = 0;
    int j = 0;
    
    char **input_cmd = malloc(1000*sizeof(char *));
    while (cmd[i] != NULL){
        if (cmd[i][0] == '>' && cmd[i][1] != '\0'){
            printf("P");
            input_cmd[j++] = ">";
        }
        // tokenize w.r.t '>'
        char *token = strtok(cmd[i], ">");
        
        if (token == NULL){
            input_cmd[j++] = cmd[i];
        }
        
        // split the command based on '>'
        int c = 0;
        while(token != NULL){
            if (c % 2 != 0){
                input_cmd[j++] = ">";
            }
            input_cmd[j++] = token;
            token = strtok(NULL, ">");
            
            c++;
        }
        i++;
    }
    input_cmd[j] = NULL;
    
    
    // counter to check # of time '>' occurs in command
    int counter = 0;
    i = 0;

    // 1 if redirection command else 0
    *isRedirection = 0;
    // store filename if redirection command
    *filename = NULL;

    while(input_cmd[i] != NULL){
        // if 1 '>' exists in command, then redirection 
        if (strcmp(input_cmd[i], ">") == 0){
            *isRedirection = 1;
            counter++;

            // if more than 1 '>' operators in command, give error
            if (counter > 1){
                *isRedirection = -1;
                break;
           }
        }
        // part of command left to '>' copied as command and right part as filename
        else if (counter == 0){
           redirection[i] = input_cmd[i];
        }
        else if (counter == 1){
           if (*filename == NULL){
              *filename = input_cmd[i];
              *filename = strtok(*filename, " \t\r\n\a");
              *isRedirection = 1;
           }
           else{
              //DisplayError();
              *isRedirection = -1;
              break;
           }
        }
        i++;
    }
    // if '>' exists but file not given, then give error
    if (*isRedirection == 1 && *filename == NULL){
        *isRedirection = -1;
    }
}

// function to execute non-redirection commands
void Execute_CMD(char **input_cmds){
    if(handle_builtin_commands(input_cmds) == 0){

        int status;
	    pid_t pid = fork();

        // child process
	    if(pid == 0){
			char **pathOptions = malloc(100*sizeof(char *));
			ParseInput(pathOptions, initialPath, "  \t\r\n\a");

			int path_flag = 0;
			int i=0;
			char *currentPathOption;

			while(pathOptions[i]!=NULL){
				    currentPathOption = (char *)malloc(strlen(input_cmds[0])+1+strlen(pathOptions[i]));
				    strcpy(currentPathOption, pathOptions[i]);
				    strcat(currentPathOption, "/");
				    strcat(currentPathOption, input_cmds[0]);
                    // check access and execv
				    if(access(currentPathOption, X_OK) == 0){
				   		path_flag = 1;
						if(execv(currentPathOption, input_cmds) == -1){
							 DisplayError();
						}
						break;
					}
				free(currentPathOption);
				i++;
			}
            // if command not found, give error
			if(path_flag == 0){
				DisplayError();
                exit(0);
			}
	    }
        // fork error
        else if (pid < 0) {
			DisplayError();
        }
        // parent process waiting child to finish
        else {
           	do {
              	waitpid(pid, &status, WUNTRACED);
          	} while (!WIFEXITED(status) && !WIFSIGNALED(status));
     	}
    }
}

// function to execute redirection commands
void Execute_Redir_CMD(char **input_cmds, char *filename){
    if(handle_builtin_commands(input_cmds) == 0){

        int status;
	    pid_t pid = fork();
	    
        // child process
	    if(pid == 0){
			char **pathOptions = malloc(100*sizeof(char *));
			ParseInput(pathOptions, initialPath, "  \t\r\n\a");
			int path_flag = 0;
			int i = 0;
			char *currentPathOption;

			while(pathOptions[i]!=NULL){
				   currentPathOption=(char *)malloc(strlen(input_cmds[0])+1+strlen(pathOptions[i]));
				   strcpy(currentPathOption, pathOptions[i]);
				   strcat(currentPathOption, "/");
				   strcat(currentPathOption, input_cmds[0]);
				   // check access, execv and copy output inside a file
				   if(access(currentPathOption, X_OK) == 0){
				   		path_flag = 1;
				        int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
				        dup2(fd, 1);
				        dup2(fd, 2);
				        close(fd);
				        execv(currentPathOption,input_cmds);
						break;
					}
				free(currentPathOption);
				i++;
			}
            // if command not found, give error
			if(path_flag == 0){
			    DisplayError();
                exit(0);
			}
	    }
        // fork error
        else if (pid < 0) {
			DisplayError();
        }
        // parent process waiting child to finish
        else {
           	do {
              	waitpid(pid, &status, WUNTRACED);
          	} while (!WIFEXITED(status) && !WIFSIGNALED(status));
     	}
    }
}

// function to handle built-in commands
int handle_builtin_commands(char **input_cmds){
    // handle empty commands
    if (input_cmds[0] == NULL){
        return 0;
    }
    // handle 'exit' command
    if(strcmp(input_cmds[0],"exit") == 0){
        
        if(input_cmds[1] != NULL){
            DisplayError();
        }
        else{
            exit(0);
        }
        return 1;
    }

    // handle 'cd' command
    else if(strcmp(input_cmds[0],"cd") == 0){
        if(input_cmds[1] == NULL){
            DisplayError();
       }
        else if(input_cmds[2] != NULL){
            DisplayError();
        }
        else{
            int check_dir_change = chdir(input_cmds[1]);
            if(check_dir_change == -1){
                DisplayError();
            }
        }
        return 1;
    }

    // handle 'path' command
    else if(strcmp(input_cmds[0],"path") == 0){
        strcpy(initialPath, "");
        int path_variable = 1;
        while(input_cmds[path_variable] != NULL){
            strcat(initialPath, input_cmds[path_variable]);
            strcat(initialPath, " ");
            path_variable++;
        }
        return 1;
    }
    return 0;
}

// function to split to command line into individual commands 
void ParseCommand(char *commands){
    // check of invalid command line
    if (commands[0] == '&'){
        DisplayError();
        return;
    }
    // split all parallel commands
    char **parallel_cmds = malloc(1000*sizeof(char *));
    ParseInput(parallel_cmds, commands, "&");

    int i = 0;
    while(parallel_cmds[i] != NULL){
    	char **command = malloc(100*sizeof(char *));
        // token the command
    	ParseInput(command, parallel_cmds[i], " \t\r\n\a");

    	char **final_command = malloc(100*sizeof(char *));
    	char *filename = malloc(100*sizeof(char));

    	int* isRedirection = (int*)malloc(sizeof(int));

        // check if the command is a redirection or not
    	CheckRedirection(final_command, command, &filename, isRedirection);

    	if(*isRedirection == 0){
    	    Execute_CMD(final_command);
    	}
    	else if (*isRedirection == 1){
            if(final_command[0] == NULL){
                DisplayError();
            }
            else{
                Execute_Redir_CMD(final_command, filename);
            }
    	}
    	else if (*isRedirection == -1 || filename == NULL){
    	    DisplayError();
    	}
        
    	free(command);
        i++;
    }
}
