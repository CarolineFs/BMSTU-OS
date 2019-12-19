#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
	pid_t child;
    pid_t child2;
	if (child == -1)
	{
		perror("Can`t fork.\n");
		exit(1);
	}
	else if ((child = fork()) == 0)
	{
		printf("Child1: pid = %d; group = %d; parent = %d\n", getpid(), getpgrp(), getppid());
		// код потомка
		sleep(2);
		printf("Child1: pid = %d; group = %d; parent = %d\n", getpid(), getpgrp(), getppid());
	}
    else if (child2 == -1)
    {
        perror("Can`t fork.\n");
    }
    else if ((child2 = fork()) == 0)
	{
		printf("Child2: pid = %d; group = %d; parent = %d\n", getpid(), getpgrp(), getppid());
		// код потомка
		sleep(2);
		printf("Child2: pid = %d; group = %d; parent = %d\n", getpid(), getpgrp(), getppid());
	}
	else
	{
		// родительский код
		printf("Parent: pid = %d; group = %d; child1 = %d; child2 = %d\n", getpid(), getpgrp(), child, child2);
	}
    return 0;
}
