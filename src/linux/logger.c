#include "logger.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void write_log(const char* message)
{
    FILE *file = fopen(LOG_FILE, "a");

    fprintf(file, message);

    fclose(file);
}

void print_subprogram_message(int subprogram_num, bool is_quit)
{
    time_t rawtime;
    struct tm * timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    const char datetime_buff[100];

    strftime(datetime_buff, sizeof(datetime_buff), "%Y-%m-%d %H:%M:%S", timeinfo);

    const char message[200];

    const char quit_message[20] = {'\0'};

    if (is_quit)
    {
        strcpy(quit_message, " (ВЫХОД)");
    }

    sprintf(message, "Подпрограмма %d%s - %s: PID: %d\n", subprogram_num, quit_message, datetime_buff, getpid());
    write_log(message);
}

void print_counter(int counter)
{
    time_t rawtime;
    struct tm * timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    const char datetime_buff[100];

    strftime(datetime_buff, 100, "%Y-%m-%d %H:%M:%S", timeinfo);

    const char message[200];

    sprintf(message, "Счетчик: %d, %s: PID: %d\n", counter, datetime_buff, getpid());
    write_log(message);
}

void print_program_greeting(void)
{
    const char message[200];

    sprintf(message, "Основная программа запущена: PID: %d\n", getpid());
    write_log(message);
}
