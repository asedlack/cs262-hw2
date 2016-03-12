#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#define NPROCS 3
#define NSEC 4

/**
Block comment hurray!
**/

int lclock(int n, int hz, int pipefd[][2])
{
	char logname[10];
	sprintf(logname, "proc%d.log", n);
	FILE * log = fopen(logname, "w");
	
	dup2(fileno(log), STDIN_FILENO);
	dup2(fileno(log), STDERR_FILENO);

	time_t start_time = time(0);
	int logical_clock = 0;

	fprintf(stderr, "Machine %d has PID %d and clockspeed %d Hz\n", n, getpid(), hz);
	fprintf(stderr, "The current time is %ld\n", time(0));

	char buf[5];

	int stat = write(pipefd[1][1], "hello", 5);
	fprintf(stderr, "Just wrote, stat is %d\n", stat);
	
	stat = read(pipefd[1][0], buf, 5);
	fprintf(stderr, "Just read, statu is %d\n", stat);


	while (time(0) < (start_time + NSEC))
	{
		fprintf(stderr, "Time is now %ld\n", time(0));
		int stat = read(pipefd[n][0], buf, 5);

		if (stat != -1){
			fprintf(stderr, "Just read, stat is %d and buf is currently %s\n", stat, buf);
		}

		usleep(1000000 / hz);
	}

	return 0;
}

int main (int argc, char** argv) // Return -1 on error
{
	//fprintf(stderr, "Woohoo, here goes the spawner with pid %ld\n", (long) getpid());
	int pipefd[NPROCS][2];	//Store our system of pips for IPC
	int hz[NPROCS];

	pid_t pid;
	int n, i, flags;

	srand((int)time(NULL) ^ (getpid()<<8));

	for (n = 0; n < NPROCS; n++)
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
		fprintf(stderr, "Pipe %d read is %d and write is %d\n", n, pipefd[n][0], pipefd[n][1]);
	}

	//Now we'll initiallize each proc before handing it off to clock()
	for (n = 0; n < NPROCS; n++)
	{
		fprintf(stderr, "Now runing on number: %d\n", n);
		
		pid = fork();

		if (pid == 0) // I'm a child
		{
			fprintf(stderr, "New proc with pid %ld\n", (long) getpid());
			

			//close uneccesaary connections
			for (i = 0; i < NPROCS; i++){
				if (i != n){
					close(pipefd[i][0]);
				}
			}
			close(pipefd[n][1]);
			
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
	
	for (i = 0; i < NPROCS; i++)
	{
		close(pipefd[i][0]);
		close(pipefd[i][1]);
	}

    return 0;
}