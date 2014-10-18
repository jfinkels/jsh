#include <stdlib.h>  /* malloc */
#include <unistd.h>  /* execv */

#include "command.h"

Command *
command_new() {
  Command * result = malloc(sizeof(Command));
  result->filename = NULL;
  result->command_line = NULL;
  return result;
}

int
command_execv (Command * command)
{
  if (command == NULL) {
    return -1;
  }
  return execv(command->filename, command->command_line);
}
