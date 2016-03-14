#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#define NPROCS 3
#define NSEC 60
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
	
	//redirect stdout and stderr to log files
	dup2(fileno(log), STDOUT_FILENO);
	dup2(fileno(log), STDERR_FILENO);

	time_t start_time = time(0);
	int logical_clock = 0;

	fprintf(stderr, "===== Machine %d has PID %d and clockspeed %d Hz =====\n", n, getpid(), hz);

	char buf[MESSAGE_SIZE];
	while (time(0) < (start_time + NSEC))
	{
		// read all available message (doesn't block)
		while(read(pipefd[n][0], buf, MESSAGE_SIZE) != -1){
			received[queue_end] = atoi(buf);
			queue_end ++;
		}

		// if a message was received
		if (queue_start < queue_end)
		{
			int message_timestamp = received[queue_start];
			logical_clock++;
			logical_clock = logical_clock > message_timestamp ? logical_clock : message_timestamp;
			queue_start++;
			int queue_size = queue_end - queue_start;
			fprintf(stdout, "EVENT[T: %ld][LC: %d] - RECIEVE: Proc %d. Queue size is %d\n", time(0), logical_clock, n, queue_size);
		}
		else
		{
			logical_clock++;
			int spinner = (rand() % 10) + 1;
			if (spinner == 1 || spinner == 2) //send message to one other proc
			{
				int recipient = (n + spinner) % NPROCS;
				snprintf(buf, sizeof(buf), "%d", logical_clock);
				write(pipefd[recipient][1], buf, MESSAGE_SIZE);
				fprintf(stdout, "EVENT[T: %ld][LC: %d] - SEND: Proc %d to proc %d\n", time(0), logical_clock, n, recipient);
			}
			else if (spinner == 3) //send message to both procs
			{
				int recipient1 = (n + 1) % NPROCS;
				int recipient2 = (n + 2) % NPROCS;
				snprintf(buf, sizeof(buf), "%d", logical_clock);
				write(pipefd[recipient1][1], buf, MESSAGE_SIZE);
				write(pipefd[recipient2][1], buf, MESSAGE_SIZE);
				fprintf(stdout, "EVENT[T: %ld][LC: %d] - SEND: Proc %d to procs %d and %d\n", time(0), logical_clock, n, recipient1, recipient2);
			}
			else //internal event
			{
				fprintf(stdout, "EVENT[T: %ld][LC: %d] - INTERNAL EVENT: Code %d\n", time(0), logical_clock, spinner);
			}
		}
		usleep(1000000 / hz);
	}

	return 0;
}

int main (int argc, char** argv) // Return -1 on error
{
	//fprintf(stderr, "Woohoo, here goes the spawner with pid %ld\n", (long) getpid());
	int pipefd[NPROCS][2];	//[proc][read socket, write socket] array to store our system of pipes for IPC
	int hz[NPROCS];

	pid_t pid;
	int n, i, flags;

	srand((int)time(NULL));
	int child_seeds[NPROCS];

	// create each of the pipes and set proc properties
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

		child_seeds[n] = (int) rand();
	}

	//Now we'll initiallize each proc before handing it off to lclock()
	for (n = 0; n < NPROCS; n++)
	{
		fprintf(stderr, "INIT: Number %d\n", n);
		
		pid = fork();

		if (pid == 0) // I'm a child
		{
			//close uneccesaary connections
			for (i = 0; i < NPROCS; i++){
				if (i != n){
					close(pipefd[i][0]);
				}
			}
			close(pipefd[n][1]);			
			lclock(n, hz[n], pipefd, child_seeds[n]);
			return 0;

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