#ifndef COMMAND_H
#define COMMAND_H

#define COMMAND_LINE_LENGTH 256
#define MAX_ARGS 256

typedef struct command_t {
  char * filename;
  char ** command_line;
} Command;

Command * command_new();
int command_execv(Command * command);

#endif
