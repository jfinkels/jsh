#include <ctype.h>  /* isspace */
#include <errno.h>  /* errno */
/*#include <stdarg.h> varargs */
#include <stdio.h>  /* printf, fgets */
#include <stdlib.h>  /* exit, atoi, getenv */
#include <string.h>  /* strrchr */
#include <sys/types.h>  /* pid_t */
#include <sys/wait.h>  /* waitpid */
#include <unistd.h>  /* fork, access */

#define MAX_SPLIT 256
#define COMMAND_LINE_LENGTH 256
#define PATH_LENGTH 256
#define PROMPT "JSH$ "
#define WHITESPACE " \t\n\v\f\r"
#define PATHSEP '/'
#define splitw(result, string) split(result, string, WHITESPACE)

/**
 * Splits `string` on the specified delimiters into an array of strings.
 *
 * `result` is an array of strings to which the result will be written.
 *
 * `string` is the string to split.
 *
 * `delimiters` the characters on which to split.
 *
 * Returns the number of strings that were created from the split.
 */
size_t split(char ** result, char * string, char * delimiters) {
  size_t j = 0;
  char * token = strtok(string, delimiters);
  while (token != NULL) {
    result[j++] = token;
    token = strtok(NULL, delimiters);
  }
  return j;
}


int endswith(char * string, char c) {
  size_t length = strlen(string);
  return string[length] == c;
}


void join(char * result, char * dirname, char * basename) {
  size_t dirname_length;
  size_t basename_length;
  dirname_length = strlen(dirname);
  basename_length = strlen(basename);
  if (endswith(dirname, PATHSEP) && dirname_length > 1) {
    dirname_length--;
  }
  strncpy(result, dirname, dirname_length);
  result[dirname_length] = PATHSEP;
  strncpy(result + dirname_length + 1, basename, basename_length);
  result[dirname_length + basename_length + 1] = '\0';
}

/**
 * Sets `result` to be the basename of the specified path.
 *
 * `path` is a path (for example `/usr/bin/make`).
 */
void basename(char * result, char * path) {
  char * base;
  size_t length;
  /** Find the last occurrence of the path separator. */
  base = strrchr(path, PATHSEP);
  /** If not found, copy the entire path. */
  if (base == NULL) {
    length = strlen(path);
    memmove(result, path, length + 1);
    return;
  }
  /** Add one to account for the separator. */
  base++;
  length = strlen(base);
  /** Add one to account for the null terminator. */
  memmove(result, base, length + 1);
}


int main() {
  int exec_result;
  int exit_status;
  int child_status;
  int access_status;
  int file_found;
  size_t argc;
  size_t path_length;
  pid_t pid;
  char raw_input[COMMAND_LINE_LENGTH];
  char * command_line[MAX_SPLIT];
  char * path[PATH_LENGTH];
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
      fprintf(stderr, "Failed to fork: (%d) %s\n", errno, strerror(errno));
    }
    /** If we are in the child process... */
    else if (pid == 0) {
      /** Check if the program location is executable. */
      access_status = access(program_location, X_OK);
      if (access_status == 0) {
        file_found = 1;
      }
      else {
        /** If the file exists but is not executable, don't search for it. */
        if (errno == EACCES) {
          fprintf(stderr, "File is not executable: %s\n", program_location);
          return 0;
        }
        /** If the file doesn't exist, iterate over each directory in PATH. */
        path_length = split(path, getenv("PATH"), ":");
        for (int i = 0; i < path_length; i++) {
          /** Join the program name with the PATH directory. */
          join(temp_filename, path[i], command_line[0]);
          /** Check if the program location is executable. */
          access_status = access(temp_filename, X_OK);
          if (access_status == 0) {
            /** Store the path to the program in program_location. */
            strncpy(program_location, temp_filename, strlen(temp_filename));
            program_location[strlen(temp_filename)] = '\0';
            file_found = 1;
            break;
          }
          else {
            /** If the file exists but is not executable, halt the search. */
            if (errno == EACCES) {
              fprintf(stderr, "File is not executable: %s\n", temp_filename);
              break;
            }
          }
        }
      }
      if (file_found) {
        /** Execute the specified command line. */
        exec_result = execv(program_location, command_line);
        if (exec_result < 0) {
          fprintf(stderr, "Failed to exec: (%d) %s\n", errno, strerror(errno));
          /** The child process should terminate because no program was run. */
          /** TODO should we return a non-0 status if there was an exec
              error? */
          return 0;
        }
      }
    }
    /** If we are in the parent process... */
    else {
      /** Wait until the child process terminates and store its exit status. */
      wait(&child_status);
    }
  } while (1);
  return 0;
}
