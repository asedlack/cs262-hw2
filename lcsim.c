#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/**
Block comment hurray!
**/

int lclock(int n)
{
	char logname[10];
	sprintf(logname, "proc%d.log", n);
	FILE * log = fopen(logname, "w");
	
	dup2(fileno(log), STDIN_FILENO);
	dup2(fileno(log), STDERR_FILENO);

	fprintf(stderr, "Process %ld now in clocks!\n", (long) getpid());
	
	sleep(10);
	return 0;
}

int main (int argc, char** argv)	//Return -1 on error
{


	//fprintf(stderr, "Woohoo, here goes the spawner with pid %ld\n", (long) getpid());
	int num_proc = 3;	//Normally set by the sim arguments
	int pipefd[2][num_proc];	//Store our system of pips for IPC

	pid_t pid;
	int n;
	int i;

	for (n = 0; n < num_proc; n++)
	{
		if (pipe(pipefd[n]) == -1)
		{
			fprintf(stderr, "Pipe creation failed.  Terminating spawner.\n");
			return -1;
		}
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
			
			lclock(n);
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
