#include "error_handlers.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void write_err_check(char *msg)
{
    // method to handle errors on write
    int bytes_written = write(STDERR_FILENO, msg, strlen(msg));
    if (bytes_written == -1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
}