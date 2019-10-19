#include "types.h"
#include "user.h"
#include "pstat.h"
#define check(exp, msg) if(exp) {} else{\
	printf(1, "%s:%d check(" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
	exit();}

void spin()
{
	int i = 0;
	int j = 0;
	int k = 0;
	for (i = 0; i < 500; i++)
	{
		for(j = 0; j < 100000; j++)
		{
			k = j % 10;
			k = k + 1;
		}
	}
}

void print(struct pstat *st)
{
	printf(1, "***********getpinfo results************\n");
	int i;
	for(i = 0; i < NPROC; i++) {
		if (st->inuse[i]) {
			printf(1, "pid: %d tickets: %d ticks: %d i: %d\n", st->pid[i], st->tickets[i], st->ticks[i], i);
		}
	}
}

int compare(int low_pid, int high_pid, struct pstat *st)
{
	int i;
	int ticks_low_pid, ticks_high_pid;
	for(i = 0; i < NPROC; i++)
	{
		if(st->pid[i] == low_pid)
			ticks_low_pid = st->ticks[i];
		else if (st->pid[i] == high_pid)
			ticks_high_pid = st->ticks[i];
	}

	return ticks_high_pid > ticks_low_pid;
}

int
main(int argc, char *argv[])
{
	int procs = 2;
	int child_pid[2];
	int parent_tickets = 100000000;
	int low_tickets = 1;
	int high_tickets = 1000;
	check(settickets(parent_tickets) == 0, "settickets");
	int pipe_1[2];  // used by child processes to alert parent they finished
	int pipe_2[2];  // used by parent process to communicate with children

	// pipe_1[0] parent uses to read
	// pipe_1[1] children use to write status
	// pipe_2[0] children use to read status
	// pipe_2[1] parent writes status to children

	if(pipe(pipe_1) < 0 || pipe(pipe_2) < 0) {
		printf(1, "pipe failed\n");
		exit();
	}

	int i;
	for(i = 0; i < procs; i++) {	// Fork 2 child processes
		child_pid[i] = fork();
		if (child_pid[i] < 0) {
			printf(1, "Fork failed!\n");
			exit();
		}

		if (child_pid[i] == 0) {	// Children processes
			close(pipe_1[0]);
			close(pipe_2[1]);
			if (i == 0) {	// set low tickets for child A
				check(settickets(low_tickets) == 0, "settickets");
				printf(1, "set low tickets for process A with pid: %d\n", getpid());
				spin();
				// Done spinning, let parent know
				write(pipe_1[1], "a", sizeof(char));
				printf(1, "A completed first round of spinning..\n");
				
				// Read from pipe -- This blocks while parent getts ready
				printf(1, "set high tickets for process A\n");
				check(settickets(high_tickets) == 0, "settickets");
				char buf[1];
				if (read(pipe_2[0], buf, sizeof(char)) < 0) {
					printf(1, "ERROR READING A\n");
					exit();
				}

				// Switch tickets
				spin();
				write(pipe_1[1], "a", sizeof(char));
				printf(1, "A completed spinning\n");

			} else {	// set high tickets for B
				check(settickets(high_tickets) == 0, "settickets");
				printf(1, "set high tickets for process B with pid: %d\n", getpid());
				spin();
				// Done
				write(pipe_1[1], "a", sizeof(char));
				printf(1, "B completed first round of spinning...\n");

				// Verify B ran longer than A
				struct pstat st;
				getpinfo(&st);
				print(&st);
				check(compare(child_pid[1], getpid(), &st) == 1, "A ticks should be less than B\n");

				// Read from pipe -- This blocks while parent gets ready

				printf(1, "set low tickets for process B\n");
				check(settickets(low_tickets) == 0, "settickets");
				char buf[1];
				if (read(pipe_2[0], buf, sizeof(char)) < 0) {
					printf(1, "ERROR READING\n");
					exit();
				}
				spin();
				write(pipe_1[1], "a", sizeof(char));
				printf(1, "B completed spinning \n");
			
			}
			exit();
		} 
	}
	close(pipe_1[1]);
	close(pipe_2[0]);
	char buf[2];
	int j;
	for(j = 0; j < procs; j++)
		if(read(pipe_1[0], buf, sizeof(char)) < 0)
			printf(1, "ERROR READING\n");

	printf(1, "Passed 1/2 tests. Switching tickets...\n");

	// Switch tickets by issuing write to pipe
	struct pstat st;
	for(j = 0; j < procs; j++)
		if(write(pipe_2[1], "a", sizeof(char)) < 0)
			printf(1, "WRITE ERROR IN PARENT\n");

	// One of the children have finished
	if (read(pipe_1[0], buf, sizeof(char)) < 0)
		printf(1, "ERROR READING 2ND\n");


	getpinfo(&st);

	printf(1, "Checking for correct values...\n");
	print(&st);
	check(compare(child_pid[1], child_pid[0], &st) == 1, "A ticks should be greater than B");

	// Block until other child has finished
	if (read(pipe_1[0], buf, sizeof(char)) < 0)
		printf(1, "ERROR READING 3RD\n");

	while(wait() > -1);
	printf(1, "Should print 1 then 2\n");
	exit();
}


