#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "port.h"
#include "logger.h"

typedef struct {
    unsigned int counter;
    int subprogram_1_finished;
    int subprogram_2_finished;
} shared_data_t;

shared_data_t *shared;

HANDLE hMap;
HANDLE counter_sem;
HANDLE sub1_sem;
HANDLE sub2_sem;
HANDLE job;

void init_shared_memory()
{
    HANDLE hMap = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        4096,
        "my_shm"
    );

    hMap = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(shared_data_t),
        "my_shm"
    );

    shared = (shared_data_t *)MapViewOfFile(
        hMap,
        FILE_MAP_ALL_ACCESS,
        0, 0,
        sizeof(shared_data_t)
    );

    counter_sem = CreateSemaphoreA(NULL, 1, 1, "counter_sem");
    sub1_sem = CreateSemaphoreA(NULL, 1, 1, "sub1_sem");
    sub2_sem = CreateSemaphoreA(NULL, 1, 1, "sub2_sem");
}

void increment_counter_process()
{
    while (1) {
        WaitForSingleObject(counter_sem, INFINITE);
        shared->counter++;
        ReleaseSemaphore(counter_sem, 1, NULL);
        Sleep(300);
    }
}

void log_counter_process()
{
    while (1) {
        WaitForSingleObject(counter_sem, INFINITE);
        print_counter(shared->counter);
        ReleaseSemaphore(counter_sem, 1, NULL);
        Sleep(1000);
    }
}

void sub_program_1_process()
{
    print_subprogram_message(1, false);

    WaitForSingleObject(sub1_sem, INFINITE);
    shared->subprogram_1_finished = 0;
    ReleaseSemaphore(sub1_sem, 1, NULL);

    WaitForSingleObject(counter_sem, INFINITE);
    shared->counter += 10;
    ReleaseSemaphore(counter_sem, 1, NULL);

    WaitForSingleObject(sub1_sem, INFINITE);
    shared->subprogram_1_finished = 1;
    ReleaseSemaphore(sub1_sem, 1, NULL);

    print_subprogram_message(1, true);
}

void sub_program_2_process()
{
    print_subprogram_message(2, false);

    WaitForSingleObject(sub2_sem, INFINITE);
    shared->subprogram_2_finished = 0;
    ReleaseSemaphore(sub2_sem, 1, NULL);

    WaitForSingleObject(counter_sem, INFINITE);
    shared->counter *= 2;
    ReleaseSemaphore(counter_sem, 1, NULL);

    Sleep(2000);

    WaitForSingleObject(counter_sem, INFINITE);
    shared->counter /= 2;
    ReleaseSemaphore(counter_sem, 1, NULL);

    WaitForSingleObject(sub2_sem, INFINITE);
    shared->subprogram_2_finished = 1;
    ReleaseSemaphore(sub2_sem, 1, NULL);

    print_subprogram_message(2, true);
}

void sub_program_process()
{
    while (1) {
        Sleep(3000);

        WaitForSingleObject(sub1_sem, INFINITE);
        WaitForSingleObject(sub2_sem, INFINITE);

        int ready = shared->subprogram_1_finished &&
                    shared->subprogram_2_finished;

        ReleaseSemaphore(sub1_sem, 1, NULL);
        ReleaseSemaphore(sub2_sem, 1, NULL);

        if (ready) {
            spawn_process("sub1");
            spawn_process("sub2");
        } else {
            write_log("Не все экземпляры подпрограммы завершили выполнение\n");
        }
    }
}

void spawn_process(const char *mode)
{
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "lab3.exe %s", mode);

    CreateProcessA(
        NULL,
        cmd,
        NULL, NULL,
        FALSE,
        CREATE_SUSPENDED,
        NULL, NULL,
        &si, &pi
    );

    AssignProcessToJobObject(job, pi.hProcess);
    ResumeThread(pi.hThread);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}

void start_main(int argc, char *argv[])
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    job = CreateJobObject(NULL, NULL);

    if (argc > 1) {
        init_shared_memory();

        if (strcmp(argv[1], "incrementer") == 0)
            increment_counter_process();
        else if (strcmp(argv[1], "logger") == 0)
            log_counter_process();
        else if (strcmp(argv[1], "sub") == 0)
            sub_program_process();
        else if (strcmp(argv[1], "sub1") == 0)
            sub_program_1_process();
        else if (strcmp(argv[1], "sub2") == 0)
            sub_program_2_process();

        return;
    }

	printf("s - запустить вручную программу, с <number> - установить значение счетчика, e - завершить\n");

    print_program_greeting();

    init_shared_memory();

    shared->counter = 0;
    shared->subprogram_1_finished = 1;
    shared->subprogram_2_finished = 1;

    spawn_process("incrementer");
    spawn_process("logger");
    spawn_process("sub");

	while(1)
	{
		char command[30];
		fgets(command, sizeof(command), stdin);

		if (command[0] == 's' || command[0] == 'S')
		{
            spawn_process("incrementer");
			printf("Вручную запущена программа\n");
		}
		
		if (command[0] == 'c' || command[0] == 'C')
		{
			char* end;
			
			long new_counter = strtol(command + 1, &end, 10);

			if (command + 1 == end) 
			{
				printf("Невалидная команда\n");
			}
			else 
			{
                WaitForSingleObject(counter_sem, INFINITE);
                shared->counter = new_counter;
                ReleaseSemaphore(counter_sem, 1, NULL);
				printf("Вручную установлено значение счетчика: %d\n", new_counter);
			}
		}

		if (command[0] == 'e' || command[0] == 'E')
		{
			break;
		}
	}

    TerminateJobObject(job, 0);
    ExitProcess(0);
}
