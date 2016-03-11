#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


/**
Block comment hurray!
**/

int lclock()
{
	printf("Process %ld now in clocks!\n", (long) getpid());
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

	//Now we'll initiallize each proc before handing it off to clock()
	for (n = 0; n < num_proc; n++)
	{
		fprintf(stderr, "Now runing on number %d\n", n);
		if (pipe(pipefd[n]) == -1)
		{
			fprintf(stderr, "Pipe creation failed.  Terminating spawner.\n");
			return -1;
		}
		pid = fork();

		if (pid == 0)	//I'm a child
		{
			fprintf(stderr, "New proc with pid %ld\n", (long) getpid());
			int spawn_status;
			waitpid(getppid(), &spawn_status, 0);

			if(spawn_status == -1)
			{
				printf("Spawning process failed.  Terminating child.\n");
				return -1;
			}

			else
			{
				for (i = 0; i < num_proc; i++){
					if (i != n){
						close(pipefd[0][i]);
					}
				}
				close(pipefd[1][n]);
				lclock();
				return 0;
			}
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
