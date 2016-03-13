#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#define NPROCS 3
#define NSEC 4
#define MAX_CLOCK_FREQUENCY 6
#define MESSAGE_SIZE 8
/**
Block comment hurray!
**/

int lclock(int n, int hz, int pipefd[][2], int seed)
{
	srand(seed);
	int received[NSEC * NPROCS * MAX_CLOCK_FREQUENCY];
	int queue_start = 0;
	int queue_end = 0;

	char log_filename[10];
	sprintf(log_filename, "proc%d.log", n);
	FILE * log = fopen(log_filename, "w");
	
	dup2(fileno(log), STDIN_FILENO);
	dup2(fileno(log), STDERR_FILENO);

	time_t start_time = time(0);
	int logical_clock = 0;

	fprintf(stderr, "Machine %d has PID %d and clockspeed %d Hz\n", n, getpid(), hz);
	fprintf(stderr, "The current time is %ld\n", time(0));

	char buf[MESSAGE_SIZE];

	while (time(0) < (start_time + NSEC))
	{
		write(pipefd[1][1], "10", MESSAGE_SIZE);
		while(read(pipefd[n][0], buf, MESSAGE_SIZE) != -1){
			received[queue_end] = atoi(buf);
			fprintf(stderr, "Just read %d\n", received[queue_end]);
			queue_end ++;
		}

		if (queue_start < queue_end)
		{
			//take message off queue, update lc, log message recieve, log global time, log queue size, and log lc time
		}
		else
		{
			logical_clock++;
			int spinner = (rand() % 10) + 1;
			if (spinner == 1 || spinner == 2)
			{
				int recipient = (n + spinner) % NPROCS;
				snprintf(buf, sizeof(buf), "%d", logical_clock);
				write(pipefd[recipient][1], buf, MESSAGE_SIZE);
				sprintf(stderr, "EVENT[%ld]: Proc %d sent to proc %d the current logical clock time %d\n", time(), n, recipient, logical_clock);
			}
			else if (spinner == 3)
			{
				int recipient1 = (n + 1) % NPROCS;
				int recpiient2 = (n + 2) % NPROCS;
				snprintf(buf, sizeof(buf), "%d", logical_clock);
				write(pipefd[recipient1][1], buf, MESSAGE_SIZE);
				write(pipefd[recipient2][1], buf, MESSAGE_SIZE);
				sprintf(stderr, "EVENT[%ld]: Proc %d sent to both procs %d and %d the current logical clock time %d\n", time(), n, recpient1, recipient2, logical_clock);
			}
			else
			{
				sprintf(stderr, "EVENT[%ld]: Internal event code %d at the logical clock time %d\n", time(), spinner, logical_clock);
			}
		}
		fprintf(stderr, "Time is now %ld\n", time(0));
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

	srand((int)time(NULL));
	int child_seeds[NPROCS];

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
 
		hz[n] = (rand() % MAX_CLOCK_FREQUENCY) + 1;
		fprintf(stderr, "Pipe %d read is %d and write is %d\n", n, pipefd[n][0], pipefd[n][1]);

		child_seeds[n] = (int) rand();
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
			
			lclock(n, hz[n], pipefd, child_seeds[n]);
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