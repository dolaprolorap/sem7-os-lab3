#define _POSIX_C_SOURCE 199309L

#include "port.h"
#include "logger.h"

#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

typedef struct {
    unsigned int counter;

    int subprogram_1_finished;
    int subprogram_2_finished;

    sem_t counter_sem;
    sem_t subprogram_1_sem;
    sem_t subprogram_2_sem;
} shared_data_t;

shared_data_t *shared;

void init_shared_memory()
{
    int fd = shm_open("/my_shm", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(shared_data_t));

    shared = mmap(NULL, sizeof(shared_data_t),
                  PROT_READ | PROT_WRITE,
                  MAP_SHARED, fd, 0);

    close(fd);

    shared->counter = 0;
    shared->subprogram_1_finished = 1;
    shared->subprogram_2_finished = 1;

    sem_init(&shared->counter_sem, 1, 1);
    sem_init(&shared->subprogram_1_sem, 1, 1);
    sem_init(&shared->subprogram_2_sem, 1, 1);
}

void increment_counter_process()
{
    struct timespec delay = {0, 300 * 1000 * 1000};

    while (1) {
        sem_wait(&shared->counter_sem);
        shared->counter++;
        sem_post(&shared->counter_sem);

        nanosleep(&delay, NULL);
    }
}

void log_counter_process()
{
    struct timespec delay = {1, 0};

    while (1) {
        sem_wait(&shared->counter_sem);
        print_counter(shared->counter);
        sem_post(&shared->counter_sem);

        nanosleep(&delay, NULL);
    }
}

void sub_program_1_process()
{
    sem_wait(&shared->subprogram_1_sem);
    shared->subprogram_1_finished = 0;
    sem_post(&shared->subprogram_1_sem);

    print_subprogram_message(1, false);

    sem_wait(&shared->counter_sem);
    shared->counter += 10;
    print_subprogram_message(1, true);
    sem_post(&shared->counter_sem);

    sem_wait(&shared->subprogram_1_sem);
    shared->subprogram_1_finished = 1;
    sem_post(&shared->subprogram_1_sem);

    _exit(0);
}

void sub_program_2_process()
{
    sem_wait(&shared->subprogram_2_sem);
    shared->subprogram_2_finished = 0;
    sem_post(&shared->subprogram_2_sem);

    print_subprogram_message(2, false);

    sem_wait(&shared->counter_sem);
    shared->counter *= 2;
    sem_post(&shared->counter_sem);

    sleep(2);

    sem_wait(&shared->counter_sem);
    shared->counter /= 2;
    sem_post(&shared->counter_sem);

    print_subprogram_message(2, true);

    sem_wait(&shared->subprogram_2_sem);
    shared->subprogram_2_finished = 1;
    sem_post(&shared->subprogram_2_sem);

    _exit(0);
}

void subprogram_process()
{
    while (1) {
        sleep(3);

        sem_wait(&shared->subprogram_1_sem);
        sem_wait(&shared->subprogram_2_sem);

        int ready = shared->subprogram_1_finished &&
                    shared->subprogram_2_finished;

        sem_post(&shared->subprogram_1_sem);
        sem_post(&shared->subprogram_2_sem);

        if (ready) {
            if (fork() == 0) sub_program_1_process();
            if (fork() == 0) sub_program_2_process();
        } else {
            write_log("Не все экземпляры подпрограммы завершили выполнение\n");
        }
    }
}

void set_counter(int value)
{
    sem_wait(&shared->counter_sem);
    shared->counter = value;
    sem_post(&shared->counter_sem);
}

void start_main_program(bool full_access)
{
    print_program_greeting();   

    init_shared_memory();

    if (fork() == 0)
        increment_counter_process();

    if (full_access) {
        if (fork() == 0)
            log_counter_process();

        if (fork() == 0)
            subprogram_process();
    }
}

void clear_memory(void)
{
    shm_unlink("/my_shm");
    kill(0, SIGTERM);
}
