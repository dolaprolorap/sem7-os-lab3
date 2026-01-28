#include "logger.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void write_log(const char* message)
{
    FILE *file = fopen(LOG_FILE, "a");

    fprintf(file, "%s", message);

    fclose(file);
}

void print_subprogram_message(int subprogram_num, bool is_quit)
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    char datetime_buff[100];
    sprintf(datetime_buff, "%04d-%02d-%02d %02d:%02d:%02d",
            st.wYear, st.wMonth, st.wDay,
            st.wHour, st.wMinute, st.wSecond);

    char quit_message[20] = "";
    if (is_quit)
    {
        strcpy(quit_message, " (ВЫХОД)");
    }

    char message[200];
    sprintf(message, "Подпрограмма %d%s - %s: PID: %lu\n",
            subprogram_num, quit_message, datetime_buff, GetCurrentProcessId());

    write_log(message);
}

void print_counter(int counter)
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    char datetime_buff[100];
    sprintf(datetime_buff, "%04d-%02d-%02d %02d:%02d:%02d",
            st.wYear, st.wMonth, st.wDay,
            st.wHour, st.wMinute, st.wSecond);

    char message[200];
    sprintf(message, "Счетчик: %d, %s: PID: %lu\n", counter, datetime_buff, GetCurrentProcessId());

    write_log(message);
}

void print_program_greeting(void)
{
    char message[200];
    sprintf(message, "Основная программа запущена: PID: %lu\n", GetCurrentProcessId());
    write_log(message);
}
