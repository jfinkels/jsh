#ifndef JSH_H
#define JSH_H

#include <string.h>  /** size_t */

size_t split(char ** result, char * string, char * delimiters);
int endswith(char * string, char c);
bool starts_with(char * string, char * initial);
void join(char * result, char * dirname, char * basename);
void basename(char * result, char * path);

#endif
