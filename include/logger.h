#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>

#define LOG_FILE "log.txt"

void print_subprogram_message(int subprogram_num, bool is_quit);
void print_counter(int counter);
void print_program_greeting(void);

#endif // LOGGER_H
