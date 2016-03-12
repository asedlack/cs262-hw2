#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#define NPROCS 3
#define NSEC 10

/**
Block comment hurray!
**/

int lclock(int n, int hz, int pipefd[][])
{
	char logname[10];
	sprintf(logname, "proc%d.log", n);
	FILE * log = fopen(logname, "w");
	
	dup2(fileno(log), STDIN_FILENO);
	dup2(fileno(log), STDERR_FILENO);

	time_t start_time = time(0);
	int logical_clock = 0;

	fprintf(stderr, "Machine %d has PID %ld and clockspeed %d Hz\n", n, getpid(), hz);
	fprintf(stderr, "The current time is %ld\n", time(0));

	char buf[5];

	if (n == 0){
		write(pipefd[1][1], "hello", 5);
	}


	while (time(0) < (start_time + NSEC))
	{
		fprintf(stderr, "Time is now %ld\n", time(0));



		usleep(1000000 / hz);
	}
	



	
	return 0;
}

int main (int argc, char** argv)	//Return -1 on error
{
	//fprintf(stderr, "Woohoo, here goes the spawner with pid %ld\n", (long) getpid());
	int num_proc = NPROCS;	//Normally set by the sim arguments
	int pipefd[2][num_proc];	//Store our system of pips for IPC
	int hz[num_proc];

	pid_t pid;
	int n, i, flags;

	srand((int)time(NULL) ^ (getpid()<<8));

	for (n = 0; n < num_proc; n++)
	{
		if (pipe(pipefd[n]) == -1)
		{
			fprintf(stderr, "Pipe creation failed.  Terminating spawner.\n");
			return -1;
		}
		flags = fcntl(pipefd[0][n], F_GETFL) | O_NONBLOCK;
		fcntl(pipefd[0][n], F_SETFL, flags);
		flags = fcntl(pipefd[1][n], F_GETFL) | O_NONBLOCK;
		fcntl(pipefd[1][n], F_SETFL, flags);
		hz[n] = (rand() % 6) + 1;
	}

	//Now we'll initiallize each proc before handing it off to clock()
	for (n = 0; n < num_proc; n++)
	{
		fprintf(stderr, "Now runing on number: %d\n", n);
		
		pid = fork();

		if (pid == 0)	//I'm a child
		{
			fprintf(stderr, "New proc with pid %ld\n", (long) getpid());
			
			for (i = 0; i < num_proc; i++){
				if (i != n){
					close(pipefd[0][i]);
				}
			}
			close(pipefd[1][n]);
			
			lclock(n, hz[n], pipefd);
			return 0;
		
			fprintf(stderr, "WE SHOULD NEVER BE HERE\n");
		}
		else if (pid == -1)
		{
			fprintf(stderr, "Fork failed.  Terminating spawner.\n");
			return -1;
		}

	}
	
	for (i = 0; i < num_proc; i++)
	{
		close(pipefd[0][i]);
		close(pipefd[1][i]);
	}

    return 0;
}