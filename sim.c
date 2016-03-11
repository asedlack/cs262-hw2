#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


/**
Block comment hurray!
**/

int clock()
{
	printf("Process %ld now in clocks!\n", (long) getpid());
	return 0;
}

int main (int argc, char** argv)	//Return -1 on error
{
	int num_proc = 3;	//Normally set by the sim arguments
	int pipefd[2][num_proc];	//Store our system of pips for IPC

	pid_t pid;
	int n;

	//Now we'll initiallize each proc before handing it off to clock()
	for (n = 0; n < num_proc; n++)
	{
		if (pipe(pipefd[n]) == -1){
			return -1;
		}
		pid = fork();

		if (pid == 0)	//I'm a child
		{
			int spawn_status;
			waitpid(getppid(), &spawn_status, 0);

			if(spawn_status == -1)
			{
				printf("Spawning process failed.  Terminating.");
				exit(1);
			}

			else
			{
				for (int i = 0; i < num_proc; i++){
					if (i != n){
						close(pipefd[0][i]);
					}
				}
				close(pipefd[1][n]);
				clock();
			}

		}
		else if (pid == -1)
		{
			return -1;
		}

	}
	for (int i = 0; i < num_proc; i++)
	{
		close(pipefd[0][i]);
		close(pipefd[1][i]);
	}

    return 0;
}
