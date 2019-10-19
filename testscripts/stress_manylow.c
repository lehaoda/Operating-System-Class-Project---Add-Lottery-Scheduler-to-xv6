#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#define check(exp, msg) if(exp) {} else {\
   printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
   exit();}
#define tol 7

void spin()
{
	int i = 0;
  int j = 0;
  int k = 0;
	for(i = 0; i < 50; ++i)
	{
		for(j = 0; j < 400000; ++j)
		{
			k = j % 10;
      k = k + 1;
    }
	}
}

void print(struct pstat *st)
{
   int i;
   for(i = 0; i < NPROC; i++) {
      if (st->inuse[i]) {
          printf(1, "pid: %d tickets: %d ticks: %d\n", st->pid[i], st->tickets[i], st->ticks[i]);
      }
   }
}

int
main(int argc, char *argv[])
{
   int numprocs = NPROC-5;
   int HIGHPID = 13;
   int pid_par = getpid();
   int numtickets = 5371, starttickets = 10000000;
   int pid_chds[numprocs];
   char buffer[numprocs+1];
   int inc = 0,i = 0;
   check(settickets(starttickets) == 0, "settickets");
   int pipe_par[2];
   int pipe_chd[2];
   pipe(pipe_par);
   pipe(pipe_chd);
   printf(1,"Goal: Achieve <%d%% error in tick difference between a single high ticket process and many low ticket processes\n",tol);
   
   for (; (pid_chds[inc] = fork()) >= 0; inc++){
     if (pid_chds[inc] == 0){
       // Child
       close(pipe_par[1]);
       close(pipe_chd[0]);
       if (inc != HIGHPID){
         // Send ready signal to parent
         if (settickets(numtickets) != 0)
           write(pipe_chd[1],"b",sizeof(char));
         else
           write(pipe_chd[1],"a",sizeof(char));
       }
       else{
         // Wait for number of tickets from parent
         read(pipe_par[0],buffer,16*sizeof(char));
         int readbuf = atoi(buffer);
         if (readbuf <= 0){
           printf(1,"Pipe error. Estimating tickets for high process\n");
           readbuf = numtickets*numprocs;
         }
         // Send acknowledgment to parent
         if (settickets(readbuf) != 0)
           write(pipe_chd[1],"b",sizeof(char));
         else
           write(pipe_chd[1],"a",sizeof(char));
       }
       for (;;)
         spin();
     }
   }
   close(pipe_par[0]);
   close(pipe_chd[1]);
   
   if (inc < 32){ // Not enough children
     printf(1,"check failed: Fork failed\n");
     goto Cleanup;
   }
   int hightickets = numtickets*(inc-1); // inc is the number of forked processes
   // Wait for ready signals from children
   for (; i < inc-1; i++){
     read(pipe_chd[0],buffer,sizeof(char));
     if (*buffer == 'b'){
       printf(1,"check failed: settickets\n");
       goto Cleanup;
     }
   }
   // Send number of tickets to high ticket child
   printf(pipe_par[1],"%d\n",hightickets);
   // Wait for acknowledgment from high ticket child
   read(pipe_chd[0],buffer,sizeof(char));
   if (*buffer == 'b'){
     printf(1,"check failed: settickets\n");
     goto Cleanup;
   }
   if (settickets(numtickets*3+1) != 0){
     printf(1,"check failed: settickets\n");
     goto Cleanup;
   }
   
   struct pstat before, after;
	
   check(getpinfo(&before) == 0, "getpinfo");
   printf(1, "\n ****PInfo before**** \n");
   print(&before);
   printf(1, "Spinning...\n");
   
   spin();
   
   check(getpinfo(&after) == 0, "getpinfo");
   check(settickets(starttickets) == 0, "settickets");
   
   printf(1, "\n ****PInfo after**** \n");
   print(&after);
   
   int highdiff = 0, lowdiff = 0, count = 0;
   int j,k;
   for (i = 0; i < inc; i++){
     for (j = 0; j < NPROC; j++){
       if (before.pid[j] == pid_chds[i]){
         for (k = 0; k < NPROC; k++){
           if (after.pid[k] == pid_chds[i]){
             if (i == HIGHPID)
               highdiff = after.ticks[k] - before.ticks[j];
             else
               lowdiff += after.ticks[k] - before.ticks[j];
             count++;
             break;
           }
         }
         break;
       }
     }
   }
   if (count < inc-1){
     // Child process appeared in the first getpinfo but was absent in the second one
     printf(1,"check failed: Lost child process\n");
     goto Cleanup;
   }
   if (highdiff < 5){
     printf(1,"check failed: Expected the process with high tickets to run\n");
     goto Cleanup;
   }
   if (lowdiff < 5){
     printf(1,"check failed: Expected the processes with low tickets to run\n");
     goto Cleanup;
   }
   //printf(1,"high difference: %d, low difference: %d\n",highdiff,lowdiff);
   int diff = highdiff-lowdiff;
   if (diff < 0)
     diff = -diff;
   printf(1,"Percent error: %d%%\n",diff*100/(highdiff+lowdiff));
   if (diff*100 >= (highdiff+lowdiff)*tol){
     printf(1,"check error: Expected the process with high tickets to get a similar number of ticks as the total number of ticks of the processes with low tickets\n");
     goto Cleanup;
   }
   printf(1, "Should print 1 then 2");
Cleanup:
   for (i = 0; i < inc; i++){
     if (pid_chds[i] > pid_par){
       kill(pid_chds[i]);
     }
     else
       break;
   }
   while (wait() > -1);
   exit();
}
