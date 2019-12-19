#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


int main()
{
	pid_t child;
    pid_t child2;
	if (child == -1)
	{
		perror("Can`t fork.");
		exit(1);
	}
	else if ((child = fork()) == 0)
	{
		// потомок
		sleep(2);
		printf("Child: pid = %d; group = %d; parent = %d\n", getpid(), getpgrp(), getppid());
	}
    else if (child2 == -1)
	{
		perror("Can`t fork.");
		exit(1);
	}
	else if ((child2 = fork()) == 0)
	{
		// потомок
		sleep(2);
		printf("Child: pid = %d; group = %d; parent = %d\n", getpid(), getpgrp(), getppid());
	}
	else
	{
		// родитель
		printf("Parent: pid = %d; group = %d; child1 = %d; child2 = %d\n", getpid(), getpgrp(), child, child2);
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
