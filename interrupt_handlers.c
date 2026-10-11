#include "interrupt_handlers.h"
#include "error_handlers.h"
#include <stdbool.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


void shell_sigint_handler(int signal)
{
    /*
    * Signal handler for receiving SIGINT (Ctrl-C)
    * We simply print the prompt again here, and record that
    * the prompt has been displayed, and therefore does not 
    * need to be printed again
    * */
    int err = errno;
    write_err_check("\n");
    write_err_check(PROMPT);
    errno = err;
}