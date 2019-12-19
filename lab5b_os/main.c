#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#define WRITERS 3
#define READERS 5

#define P -1
#define V 1

#define ACT_READER 0
#define BIN_ACT_WRITER 1
#define WAIT_WRITER 2


static const int PERM =  S_IRWXU | S_IRWXG | S_IRWXO;
    // S_IRWXU - владелец имеет права на чтение, запись и выполнение файла
    // S_IRWXG - группа имеет права на чтение, запись и выполнение файла
    // S_IRWXO - все остальные (вне группы) имеют права на чтение, запись и выполнение файла

static struct sembuf start_read[] = {  // Структура, описывающая операцию над одним семафором
    {WAIT_WRITER, 0, 1},      // вызвавший процесс будет заблокирован, пока этот семафор не станет = 0
    {BIN_ACT_WRITER,  0, 1},
    {ACT_READER,  V, 1}};

static struct sembuf  stop_read[] = {
    {ACT_READER, P, 1}};

static struct sembuf  start_write[] = {
    {WAIT_WRITER,     V, 1},
    {ACT_READER,      0, 1},
    {BIN_ACT_WRITER,  P, 1},
    {WAIT_WRITER,     P, 1}};

static struct sembuf  stop_write[] = {
    {BIN_ACT_WRITER, V, 1}};


void writer(int semid, int* shm, int num)
{
    while (1)
    {
        semop(semid, start_write, 4); // производит операции над выбранными элементами из набора семафоров semid.
        (*shm)++;
        printf("process %d   Writer #%d ----> %d\n", getpid(),num, *shm);
        semop(semid, stop_write, 1);
        sleep(2);
    }
}

void reader(int semid, int* shm, int num)
{
    while (1)
    {
        semop(semid, start_read,3);
        printf("\tprocess %d  Reader #%d <---- %d\n",getpid(), num, *shm);
        semop(semid, stop_read,1);
        sleep(1);
    }
}

int main() {

    int shm_id;
    if ((shm_id = shmget(IPC_PRIVATE, 4, IPC_CREAT | PERM)) == -1)
        // возвращает идентификатор разделяемому сегменту памяти, соответствующий значению аргумента key.
    {
        perror("Unable to create a shared area.\n");
        exit(1);
    }

    int *shm_buf = (int*)shmat(shm_id, 0, 0);
        // Функция shmat подстыковывает сегмент разделяемой памяти shmid к адресному пространству вызывающего процесса.
    if (shm_buf == (void*) -1)
    {
        perror("Can't attach memory");
        exit(1);
    }

    (*shm_buf) = 0;

    int sem_id;
    if ((sem_id = semget(IPC_PRIVATE, 4, IPC_CREAT | PERM)) == -1)
        // shmget возвращает идентификатор набора семафоров, связанный с аргументом key.
    {
        perror("Unable to create a semaphore.\n");
        exit(1);
    }

    int ctrl = semctl(sem_id, BIN_ACT_WRITER, SETVAL, 1);
    if ( ctrl == -1)
    {
        perror( "Can't set semaphor`s values." );
        exit(1);
    }

    pid_t pid = -1;

    for (int i = 0; i < WRITERS && pid != 0; i++)
    {
        pid = fork();
        if (pid == -1)
        {
            perror("Can`t fork writer.\n");
            exit(1);
        }
        if (pid == 0)
        {
            writer(sem_id, shm_buf, i);
        }
    }

    for (int i = 0; i < READERS && pid != 0; i++)
    {
        pid = fork();
        if (pid == -1)
        {
            perror("Can`t fork reader.\n");
            exit( 1 );
        }
        if (pid == 0)
        {
            reader(sem_id, shm_buf, i);
        }
    }

    if (shmdt(shm_buf) == -1)
    {
        perror( "Can't detach shared memory" );
        exit(1);
    }

    if (pid != 0)
    {
        int *status;
        for (int i = 0; i < WRITERS + READERS; ++i)
        {
            wait(status);
        }
        if (shmctl(shm_id, IPC_RMID, NULL) == -1)
        {
            perror( "Can't free memory!" );
            exit(1);
        }
    }

    return 0;
}
