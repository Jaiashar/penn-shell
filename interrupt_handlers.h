#include <sys/types.h>
#include <stdbool.h>

#ifndef INTERRUPT_HANDLERS_H
#define INTERRUPT_HANDLERS_H
extern volatile bool prompted;
void shell_sigint_handler(int signal) ;
#endif
