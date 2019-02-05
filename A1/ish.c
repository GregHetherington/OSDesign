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

char * shouldOutputRedirect(char** commandArgs) {
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

int shouldInputRedirect(char** commandArgs) {
  int i = 0;
  while(commandArgs[i] != NULL) {
    if (strcmp(commandArgs[i], "<") == 0) {
      commandArgs[i] = commandArgs[i + 1];
      commandArgs[i + 1] = NULL;
      return 1;
    }
    i++;
  }
  return 0;
}

void executeCommand(char** commandArgs) {
  int i = 0;
  pid_t pid;
  int status;
  int NumOfCmds = 4;
  int SelectedCmd = -1;
  char* ListOfCommands[NumOfCmds];

  int runInBackground = shouldRunInBackground(commandArgs);

  ListOfCommands[0] = "exit";
  ListOfCommands[1] = "gcd";
  ListOfCommands[2] = "args";
  ListOfCommands[3] = "mult";

  for (i = 0; i < NumOfCmds; i++) {
    if (strcmp(commandArgs[0], ListOfCommands[i]) == 0) {
      SelectedCmd = i;
      break;
    }
  }

  if (SelectedCmd == 0) {
    exit(0);
  } else if (SelectedCmd == 1) {
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
  } else if (SelectedCmd == 2) {
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

  } else if (SelectedCmd == 3) {
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
      int inputRedirect = shouldInputRedirect(commandArgs);

      if (redirectLocation != NULL && inputRedirect == 0) {
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
      } else if (redirectLocation != NULL && inputRedirect == 1) {
        printf("Bad Command: use one of < or >\n");
      } else {
        executeCommand(commandArgs);
      }
    }

    for(i = 0; i < numOfArgs; i++) {
      free(commandArgs[i]);
    }

  }

  return 0;
}

void sigFunction(int sig) {
  int status;
  wait(&status);
}
