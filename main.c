#include <errno.h>  /* errno */
#include <stdbool.h>  /* bool */
#include <stdio.h>  /* printf, fgets, perror */
#include <stdlib.h>  /* exit, atoi, getenv */
#include <sys/types.h>  /* pid_t */
#include <sys/wait.h>  /* waitpid */
#include <unistd.h>  /* fork, access */

#include "jsh.h"

#define MAX_SPLIT 256
#define COMMAND_LINE_LENGTH 256
#define PATH_LENGTH 256
#define PROMPT "JSH$ "
#define WHITESPACE " \t\n\v\f\r"
#define splitw(result, string) split(result, string, WHITESPACE)

int main() {
  int exec_result;
  int exit_status;
  int child_status;
  int access_status;
  bool file_found = false;
  size_t argc;
  size_t i;
  size_t path_length;
  pid_t pid;
  char raw_input[COMMAND_LINE_LENGTH];
  char * command_line[MAX_SPLIT];
  char * path[PATH_LENGTH];
  char * env_path;
  char program_location[COMMAND_LINE_LENGTH];
  char temp_filename[COMMAND_LINE_LENGTH];

  do {
    /** Print the prompt. */
    printf(PROMPT);
    /** Read the command line from standard input. */
    fgets(raw_input, COMMAND_LINE_LENGTH, stdin);
    /** Split the command line into a NULL-terminated array of strings. */
    argc = splitw(command_line, raw_input);
    command_line[argc] = NULL;
    /** If the command line is empty... */
    if (argc == 0) {
      /** Simply continue to the next iteration of the loop. */
      continue;
    }
    /** If the command is an exit command... */
    if (strcmp(command_line[0], "exit") == 0) {
      exit_status = 0;
      /** If an exit status was provided as the second argument, use that. */
      if (argc > 1) {
        exit_status = atoi(command_line[1]);
      }
      return exit_status;
    }
    /**
     * Store the full location to the program as specified in the first
     * argument of the command line.
     */
    strncpy(program_location, command_line[0], strlen(command_line[0]));
    program_location[strlen(command_line[0])] = '\0';
    /**
     * Replace the first argument, the name of the command, with just its
     * basename (without the leading directory part of the path).
     *
     * According to the GNU libc manual, "By convention, the first element of
     * this array is the file name of the program sans directory names."
     */
    basename(command_line[0], command_line[0]);
    /** Fork this process into two processes. In each of the two processes, pid
        is set to the pid of that process. */
    pid = fork();
    /** If there is a fork failure... */
    if (pid < 0) {
      perror("Failed to fork");
    }
    /** If we are in the child process... */
    else if (pid == 0) {
      /** Check if the program location is executable. */
      access_status = access(program_location, X_OK);
      if (access_status == 0) {
        file_found = true;
      }
      else {
        /** If the file exists but is not executable, don't search for it. */
        if (errno == EACCES) {
          perror("File is not executable");
          return 0;
        }
          /** If the file doesn't exist, iterate over each directory in PATH,
              if PATH is not empty. */
        env_path = getenv("PATH");
        if (env_path != NULL) {
          path_length = split(path, env_path, ":");
          for (i = 0; i < path_length; i++) {
            /** Join the program name with the PATH directory. */
            join(temp_filename, path[i], command_line[0]);
            /** Check if the program location is executable. */
            access_status = access(temp_filename, X_OK);
            if (access_status == 0) {
              /** Store the path to the program in program_location. */
              strncpy(program_location, temp_filename, strlen(temp_filename));
              program_location[strlen(temp_filename)] = '\0';
              file_found = true;
              break;
            }
            else {
              /** If the file exists but is not executable, halt the search. */
              if (errno == EACCES) {
                perror("File is not executable");
                break;
              }
            }
          }
        }
      }
      if (file_found) {
        /** Execute the specified command line. */
        exec_result = execv(program_location, command_line);
        if (exec_result < 0) {
          perror("Failed to exec");
          /** The child process should terminate because no program was run. */
          /** TODO should we return a non-0 status if there was an exec
              error? */
          return 0;
        }
      } else {
        printf("%s: command not found.\n", command_line[0]);
        return -1;
      }
    }
    /** If we are in the parent process... */
    else {
      /** Wait until the child process terminates and store its exit status. */
      wait(&child_status);
    }
  } while (true);
  return 0;
}
