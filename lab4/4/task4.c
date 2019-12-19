#include <stdio.h> //printf
#include <stdlib.h> //exit
#include <unistd.h> //pipe
#include <string.h> //strlen
#include <sys/wait.h>


int main()
{
	int descr[2];   // дескриптор одного программного канала
	                // [0] - выход для чтения, [1] - выход для записи
	                // потомок унаследует открытый программный канал предка

	if (pipe(descr) == -1)
	{
        perror("Can`t pipe.");
		exit(1);
	}

    pid_t child1 = fork();
    if (child1 == -1)
	{
        perror("Couldn't fork.");
		exit(1);
	}
    if (child1 == 0)
	{
		// потомок
        //sleep(10);
        char msg1[] = "child1\n";
        close(descr[0]);

        write(descr[1], msg1, 64);

        exit(0);
	}

    int child2 = fork();
    if (child2 == -1)
    {
        perror("Couldn't fork.");
        exit(1);
    }
    if (child2 == 0)
    {
        // потомок
        char msg2[] = "child2\n";
        close(descr[0]);

        write(descr[1], msg2, 64);
        exit(0);
	}
	
	if (child1 != 0 && child2 != 0)
	{
		// родитель
		close(descr[1]);
		char msg1[64];
		read(descr[0], msg1, 64);

		char msg2[64];
		read(descr[0], msg2, 64);

		printf("Parent: reads \n %s %s", msg1, msg2);

		int status;
		pid_t ret_value;
		while ((ret_value = wait(&status)) != -1)
        {
		    if (WIFEXITED(status)) // WIFEXITED ненулевой, если дочерний процесс завершен нормально
		        printf("Parent: child %d finished with %d code.\n", ret_value, WEXITSTATUS(status));
                // Если WIFEXITED ненулевой, WEXITSTATUS возвращает код завершения дочернего процесса
		    else if (WIFSIGNALED(status))
		        printf("Parent: child %d finished from signal with %d code.\n", ret_value, WTERMSIG(status));
                // WIFSIGNALED ненулевой, если дочерний процесс завершается неперехватываемым сигналом
                // Если WIFSIGNALED ненулевой, WTERMSIG возвращает номер сигнала
		    else if (WIFSTOPPED(status))
		        printf("Parent: child %d finished from signal with %d code.\n", ret_value, WSTOPSIG(status));
                // WIFSTOPPED ненулевой, если дочерний процесс остановился
                // Если WIFSTOPPED ненулевой, WSTOPSIG возвращает номер сигнала
        }

    }

    return 0;
}
