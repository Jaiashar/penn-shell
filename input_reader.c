#include "input_reader.h"
#include "interrupt_handlers.h"
#include "error_handlers.h"
#include <unistd.h>
#include<stdio.h>
#include<stdlib.h>

char *read_input_from_prompt()
{
  const int maxLineLength = 4096;
  char *cmd = malloc(maxLineLength * sizeof(char));

  int bytes_read = read(STDIN_FILENO, cmd, maxLineLength);
  if (bytes_read < 0)
  {
    // handling read failure
    perror("read");
    exit(EXIT_FAILURE);
  }
  if (bytes_read == 0)
  {
    // exit if the user hits only Ctrl-D on an empty prompt
    write_err_check("\n");
    exit(EXIT_SUCCESS);
  }
  // some user input was obtained at this point, and therefore
  // to obtain user input in the future, the prompt must be displayed

  // null terminate the input based on whether the last character was
  // a newline or not
  if (cmd[bytes_read - 1] == '\n')
    cmd[bytes_read - 1] = '\0';
  else
  {
    cmd[bytes_read] = '\0';
    write_err_check("\n");
  }
  return cmd;
}

char *read_input_from_file()
{
  size_t buffer_size = 0;
  char *line = NULL;
  if (getline(&line, &buffer_size, stdin) == -1)
  {
    exit(EXIT_FAILURE);
  }
  return line;
}