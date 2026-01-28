#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    hMap = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(shared_data_t),
        "Global\\my_shm"
    );

    shared = (shared_data_t *)MapViewOfFile(
        hMap,
        FILE_MAP_ALL_ACCESS,
        0, 0,
        sizeof(shared_data_t)
    );

    counter_sem = CreateSemaphoreA(NULL, 1, 1, "Global\\counter_sem");
    sub1_sem    = CreateSemaphoreA(NULL, 1, 1, "Global\\sub1_sem");
    sub2_sem    = CreateSemaphoreA(NULL, 1, 1, "Global\\sub2_sem");

    shared->counter = 0;
    shared->subprogram_1_finished = 1;
    shared->subprogram_2_finished = 1;
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
        printf("Counter: %u\n", shared->counter);
        ReleaseSemaphore(counter_sem, 1, NULL);
        Sleep(1000);
    }
}

void sub_program_1_process()
{
    WaitForSingleObject(sub1_sem, INFINITE);
    shared->subprogram_1_finished = 0;
    ReleaseSemaphore(sub1_sem, 1, NULL);

    printf("Subprogram 1 start\n");

    WaitForSingleObject(counter_sem, INFINITE);
    shared->counter += 10;
    ReleaseSemaphore(counter_sem, 1, NULL);

    printf("Subprogram 1 end\n");

    WaitForSingleObject(sub1_sem, INFINITE);
    shared->subprogram_1_finished = 1;
    ReleaseSemaphore(sub1_sem, 1, NULL);
}

void sub_program_2_process()
{
    WaitForSingleObject(sub2_sem, INFINITE);
    shared->subprogram_2_finished = 0;
    ReleaseSemaphore(sub2_sem, 1, NULL);

    printf("Subprogram 2 start\n");

    WaitForSingleObject(counter_sem, INFINITE);
    shared->counter *= 2;
    ReleaseSemaphore(counter_sem, 1, NULL);

    Sleep(2000);

    WaitForSingleObject(counter_sem, INFINITE);
    shared->counter /= 2;
    ReleaseSemaphore(counter_sem, 1, NULL);

    printf("Subprogram 2 end\n");

    WaitForSingleObject(sub2_sem, INFINITE);
    shared->subprogram_2_finished = 1;
    ReleaseSemaphore(sub2_sem, 1, NULL);
}

void spawn_process(const char *mode)
{
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "program.exe %s", mode);

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

BOOL WINAPI console_handler(DWORD sig)
{
    if (sig == CTRL_C_EVENT) {
        TerminateJobObject(job, 0);
        ExitProcess(0);
    }
    return TRUE;
}

int main(int argc, char *argv[])
{
    SetConsoleCtrlHandler(console_handler, TRUE);

    job = CreateJobObject(NULL, NULL);

    if (argc > 1) {
        init_shared_memory();

        if (strcmp(argv[1], "incrementer") == 0)
            increment_counter_process();
        else if (strcmp(argv[1], "logger") == 0)
            log_counter_process();
        else if (strcmp(argv[1], "sub1") == 0)
            sub_program_1_process();
        else if (strcmp(argv[1], "sub2") == 0)
            sub_program_2_process();

        return 0;
    }

    init_shared_memory();

    spawn_process("incrementer");
    spawn_process("logger");

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
            printf("Subprograms still running\n");
        }
    }
}
