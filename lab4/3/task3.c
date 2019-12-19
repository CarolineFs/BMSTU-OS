#include <stdio.h> //printf
#include <stdlib.h> //exit
#include <unistd.h> //execlp
#include <sys/types.h>
#include <sys/wait.h>


int main()
{
	pid_t child1 = fork();
	if (child1 == -1)
	{
        perror("Can`t fork.");
		exit(1);
	}
	
	if (child1 == 0)
	{
		// потомок
		printf("Child: pid = %d; group = %d; parent = %d\n\n", getpid(), getpgrp(), getppid());
		if (execlp("ps", "ps", "al", 0) == -1)
		{
		    perror( "Child can`t exec.");
			exit(1);
        }
	}

	pid_t child2 = fork();
	if (child2 == -1)
	{
        perror("Couldn't fork.");
		exit(1);
	}
	
	if (child2 == 0)
	{
		// потомок
		printf("Child: pid = %d; group = %d; parent = %d\n\n", getpid(), getpgrp(), getppid());
		if (execlp("/bin/ls", "ls", "-l", 0) == -1)
		{
		    perror( "Child can`t exec." );
			exit(1);
        }
	}
	
	if (child1 != 0 && child2 != 0)
	{
		// родитель
		printf("Parent: pid = %d; group = %d; child1 = %d; child2 = %d\n", getpid(), getpgrp(), child1, child2);
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
