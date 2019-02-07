#define _XOPEN_SOURCE 500
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>

void sigFunction(int sig);
int getInput(char* str);
int shouldRunInBackground(char** commandArgs);
char* shouldOutputRedirect(char** commandArgs);
char* shouldInputRedirect(char** commandArgs);
void executeCommand(char** commandArgs);
void shellPrompt();

/*
  Main thread

  param: none
  return: int = 1 for error, 0 for success
*/
int main() {
  //Setup
  char inputString[64];
  char *commandArgs[10];
  int i = 0;
  //Welcome Message
  printf("\nWelcome to Greg's Shell\n\n");

  while (1) {
    /* Shell Prompt */
    shellPrompt();

    /* Get Input */
    if (getInput(inputString)) {
      continue;
    }

    /* Parse input */
    char *token, *string;
    int numOfArgs = 0;
    string = strdup(inputString);
    while (( token = strsep (&string, " ")) != NULL) {
      commandArgs[numOfArgs] = (char*)malloc(sizeof(token));
      strcpy(commandArgs[numOfArgs], token);
      numOfArgs++;
    }
    commandArgs[numOfArgs] = NULL;

    /* Run Command */
    if (numOfArgs > 0) {
      char * redirectLocation = shouldOutputRedirect(commandArgs);
      char * inputRedirect = shouldInputRedirect(commandArgs);

      if (redirectLocation != NULL && inputRedirect == NULL) {
        int pid, status;
        if ((pid = fork()) == -1) {
          printf("ERROR: forking failed\n");
          exit(1);
        } else if (pid == 0) {
          freopen(redirectLocation, "w+", stdout);
          executeCommand(commandArgs);
          exit(0);
        } else {
          wait(&status);
        }
      } else if (redirectLocation == NULL && inputRedirect != NULL) {
        int pid, status;
        if ((pid = fork()) == -1) {
          printf("ERROR: forking failed\n");
          exit(1);
        } else if (pid == 0) {
          freopen(inputRedirect, "r", stdin);
          executeCommand(commandArgs);
          exit(0);
        } else {
          wait(&status);
        }
      } else if (redirectLocation != NULL && inputRedirect != NULL) {
        printf("Bad Command: use one of < or >\n");
      } else {
        executeCommand(commandArgs);
      }
    }
    //free memory
    for(i = 0; i < numOfArgs; i++) {
      free(commandArgs[i]);
    }
  }

  return 0;
}

/*
  Signal function calls wait on async calls

  param: sig = Imput signal
  return: void
*/
void sigFunction(int sig) {
  int status;
  wait(&status);
}

/*
  Reads user input from terminal

  param: str = Input string
  return: int 0 if successful or 1 if not
*/
int getInput(char* str) {
    char inputString[64];

    fgets(inputString, 64, stdin);
    strtok(inputString, "\n");//removes newline char
    if (strlen(inputString) != 0) {
      strcpy(str, inputString);
      return 0;
    } else {
      return 1;
    }
}

/*
  Checks the command arguments for an '&' character
  to see if the command should be run in the background

  param: string array of command arguments
  return: int 1 if yes or 0 if no, to run in the background
*/
int shouldRunInBackground(char** commandArgs) {
  int i = 0;
  while(commandArgs[i] != NULL) {
    if (strcmp(commandArgs[i], "&") == 0) {
      commandArgs[i] = NULL;
      return 1;
    }
    i++;
  }
  return 0;
}

/*
  Checks the command arguments for an '>' character
  to see if the command should output to a file

  param: string array of command arguments
  return: string "fileName" if yes, NULL if no
*/
char* shouldOutputRedirect(char** commandArgs) {
  int i = 0;
  while(commandArgs[i] != NULL) {
    if (strcmp(commandArgs[i], ">") == 0) {
      commandArgs[i] = NULL;
      return commandArgs[i + 1] ; //output should redirect from file
    }
    i++;
  }
  return NULL;
}

/*
  Checks the command arguments for an '<' character
  to see if the command arguments should be Read
  from a file

  param: string array of command arguments
  return: string "fileName" if yes, NULL if no
*/
char* shouldInputRedirect(char** commandArgs) {
  int i = 0;
  while(commandArgs[i] != NULL) {
    if (strcmp(commandArgs[i], "<") == 0) {
      commandArgs[i] = commandArgs[i + 1];
      commandArgs[i + 1] = NULL;
      return commandArgs[i];
    }
    i++;
  }
  return NULL;
}

/*
  Executes Command based on command arguments
  (hope this is decriptive enough)

  param: string array of command arguments
  return: void
*/
void executeCommand(char** commandArgs) {
  int i = 0;
  pid_t pid;
  int status;
  int numOfCmds = 4;
  int selectedCmd = -1;
  char* listOfCommands[numOfCmds];

  int runInBackground = shouldRunInBackground(commandArgs);

  listOfCommands[0] = "exit";
  listOfCommands[1] = "gcd";
  listOfCommands[2] = "args";
  listOfCommands[3] = "mult";

  for (i = 0; i < numOfCmds; i++) {
    if (strcmp(commandArgs[0], listOfCommands[i]) == 0) {
      selectedCmd = i;
      break;
    }
  }

  if (selectedCmd == 0) {
    exit(0);
  } else if (selectedCmd == 1) {
    if (commandArgs[1] != NULL && commandArgs[2] != NULL) {
      long num1, num2;

      //check if Numbers are in Hex
      if (strstr(commandArgs[1], "0x")) {
        num1 = strtol(commandArgs[1], NULL, 16);
      } else {
        num1 = strtol(commandArgs[1], NULL, 10);
      }
      if (strstr(commandArgs[2], "0x")) {
        num2 = strtol(commandArgs[2], NULL, 16);
      } else {
        num2 = strtol(commandArgs[2], NULL, 10);
      }

      //Find gcd
      while (num1 != num2) {
        if (num1 > num2) {
          num1 -= num2;
        } else {
          num2 -= num1;
        }
      }
      printf("GCD(%s, %s) = %ld\n", commandArgs[1], commandArgs[2], num1);
    } else {
      printf("ERROR: Bad Arguments\n");
    }
  } else if (selectedCmd == 2) {
    int argCounter = 0;
    for (i = 1; commandArgs[i] != NULL; i++) {
      argCounter++;
    }

    printf("argc = %d, args = ", argCounter);
    for (i = 1; commandArgs[i] != NULL; i++) {
      if (i == argCounter) {
        printf("%s\n", commandArgs[i]);
      } else {
        printf("%s, ", commandArgs[i]);
      }
    }

  } else if (selectedCmd == 3) {
    if (commandArgs[1] != NULL && commandArgs[2] != NULL) {
      long num1, num2, ans;

      if (strstr(commandArgs[1], "0x")) {
        num1 = strtol(commandArgs[1], NULL, 16);
      } else {
        num1 = strtol(commandArgs[1], NULL, 10);
      }
      if (strstr(commandArgs[2], "0x")) {
        num2 = strtol(commandArgs[2], NULL, 16);
      } else {
        num2 = strtol(commandArgs[2], NULL, 10);
      }

      ans = num1 * num2;

      printf("MULT(%s, %s) = %ld\n", commandArgs[1], commandArgs[2], ans);
    } else {
      printf("ERROR: Bad Arguments\n");
    }
  } else {
    pid = fork();
    if (runInBackground == 1) {
      sigset(pid, sigFunction);
    }
    if (pid == -1) {
      printf("ERROR: forking failed\n");
      exit(1);
    } else if (pid == 0) {
      execvp(commandArgs[0], commandArgs);
    } else {
      if (runInBackground == 1) {
        printf("background job created: %d\n", pid);
      } else {
        wait(&status);
      }
    }
  }
}

/*
  Prompts the user with a message based on who
  is using this shell

  param: none
  return: void
*/
void shellPrompt() {
  char currentWorkingDir[1024];
  getcwd(currentWorkingDir, sizeof(currentWorkingDir));

  struct passwd *passwd;
  passwd = getpwuid(getuid());

  char hostName[1024];
  gethostname(hostName, sizeof(hostName));

  char userType;
  if (strcmp(passwd->pw_name, "root")) {
    userType = '$';
  } else {
    userType = '#';
  }
  printf("%s@%s:~%s%c", passwd->pw_name, hostName, currentWorkingDir, userType);
}
