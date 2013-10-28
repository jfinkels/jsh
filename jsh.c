#include <string.h>  /* strrchr, size_t */

#define PATHSEP '/'

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


/**
 * Returns a non-zero value if and only if `string` ends with character `c`.
 */
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
