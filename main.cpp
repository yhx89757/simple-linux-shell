#include <vector>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h> // for strtok()
using namespace std;

bool isBuiltInCommand(vector<char*>& args);
void builtIn_command(vector<char*>& args);

int main(int argc, char* argv[]) {
	ifstream file;
	bool batch_mode = false;
	if (argc == 2) {
		batch_mode = true;
		file.open(argv[1], ifstream::in);
	}
    while (1) {
		char cmdLine[1024];
		if (batch_mode == false) {
			// print prompt
		    cout << "mysh> ";
			// read commandline
		    cin.getline(cmdLine, 1024);
		} else {
			string cmdLine_string = "";
			if (getline(file, cmdLine_string)) {
				strcpy(cmdLine, cmdLine_string.c_str());
				write(STDOUT_FILENO, cmdLine, strlen(cmdLine));
				cout << endl;
			} else {
				file.close();
				exit(0);
			}
		}
		// if command is too long (over 512 bytes)
		if (strlen(cmdLine) > 512) {
			char error_message[30] = "An error has occurred\n";
			write(STDERR_FILENO, error_message, strlen(error_message));
			continue;
		}
        // parse and tokenize the line and save the tokens
        char* tokens = strtok(cmdLine, " "); 
		// ----if it is empty
        if (tokens == NULL) {
            continue;
        }
 		// ----if it is not empty
        vector<char*> args = vector<char*>();
        while (tokens != NULL) {
            args.push_back(tokens);
            tokens = strtok(NULL, " ");
        }
		// ----every command executed by "execvp" should end with a real
		// ----value NULL, otherwise will cause error
		args.push_back(NULL);
        // check built-in or external command
		if (isBuiltInCommand(args)) {
			builtIn_command(args);
		} else {
			pid_t pid = fork();
	    	if (pid == 0) { // child process
				if (execvp (args[0], &args[0]) == -1) {
					//perror ("exec");
					char error_message[30] = "An error has occurred\n";
					write(STDERR_FILENO, error_message, strlen(error_message));
				}
				exit(0); // we need to kill this process in the end if it is child
	    	}
	    	if (pid > 0) { // parent process
				if (wait(0) == -1) {
					//perror ("wait");
					char error_message[30] = "An error has occurred\n";
					write(STDERR_FILENO, error_message, strlen(error_message));
				}
	    	}
		}
    }
    
    return 0;
}

bool isBuiltInCommand(vector<char*>& args) {
	return !strcmp(args[0], "cd") || !strcmp(args[0], "exit") ||
		!strcmp(args[0], "pwd");
}

void builtIn_command(vector<char*>& args) {
	if (!strcmp(args[0], "exit")) {
		exit(0);
	}
	if (!strcmp(args[0], "cd")) {
		if (args[1] != NULL) { // for cd something
			if (chdir(args[1]) != 0) {
				//perror("chdir");
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message));
			}
		} else { // for just cd
			if (chdir(getenv("HOME")) != 0) {
				//perror("chdir");
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message));
			}
		}
	}
	if (!strcmp(args[0], "pwd")) {
		char pwd[255];
		if(!getcwd(pwd, sizeof(pwd))){
			//perror("getcwd");
			char error_message[30] = "An error has occurred\n";
			write(STDERR_FILENO, error_message, strlen(error_message));
			return;
		}
		cout << pwd << endl;
	}
	return;
}
