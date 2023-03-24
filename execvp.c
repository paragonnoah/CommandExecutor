#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include<string.h>
#include <sys/wait.h>

int
main(void)
{
	pid_t	pid;
	int		status;

	char buf[10][10];
	char *buf2[10];
	buf[0][0] = 'l';
	buf[0][1] = 's';
	buf[0][2] = '\0';
        buf[1][0] = '.';
        buf[1][1] = '.';
        buf[1][2] = '\0';

	buf2[0] = (char *)&buf[0][0];
	buf2[1] = (char *)&buf[1][0];
	buf2[2] = (char *)0;
	


		if ((pid = fork()) < 0) {
			printf("fork error");
		} else if (pid == 0) {		/* child */
			execvp(buf2[0], buf2);
			printf("couldn't execute: %s", buf2[0]);
			exit(127);
		}

		/* parent */
		if ((pid = waitpid(pid, &status, 0)) < 0)
			printf("waitpid error");
		printf("DONE ");
	exit(0);
}
