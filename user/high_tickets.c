#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#define check(exp, msg) if(exp) {} else {\
   printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
   exit();}

void spin()
{
	int i = 0;
  int j = 0;
  int k = 0;
	for(i = 0; i < 500; ++i)
	{
		for(j = 0; j < 200000; ++j)
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

void compare(int pid_low, int pid_high, struct pstat *before, struct pstat *after)
{
	int i, ticks_low_before=-1, ticks_low_after=-1, ticks_high_before=-1, ticks_high_after=-1;
	for(i = 0; i < NPROC; i++)
	{
    if (before->pid[i] == pid_low)
    {
        ticks_low_before = before->ticks[i];
    }
    if (before->pid[i] == pid_high)
    {
        ticks_high_before = before->ticks[i];
    }
    if (after->pid[i] == pid_low)
    {
        ticks_low_after = after->ticks[i];
    }
    if (after->pid[i] == pid_high)
    {
        ticks_high_after = after->ticks[i];
    }
  }
  printf(1, "high: %d %d, low: %d %d\n", ticks_high_before, ticks_high_after, ticks_low_before, ticks_low_after);
  check(ticks_low_before <= 200, "The parent process should not get too many ticks before child get scheduled");
  check(ticks_low_before >=0, "Ticks number in pinfo should be correct");
  check(ticks_low_after >=0, "Ticks number in pinfo should be correct");
  check(ticks_high_before >=0, "Ticks number in pinfo should be correct");
  check(ticks_high_after >=0, "Ticks number in pinfo should be correct");

	check(ticks_high_after-ticks_high_before > (ticks_low_after - ticks_low_before)*20, "Expected the process with high tickets to get more ticks than the process with low tickets");
}

int
main(int argc, char *argv[])
{
   int pid_low = getpid();
   int lowtickets = 5, hightickets = 100000;
   check(settickets(lowtickets) == 0, "settickets");

   if(fork() == 0)
   {
   	    check(settickets(hightickets) == 0, "settickets");
        int pid_high = getpid();
        struct pstat st_before, st_after;
	
        check(getpinfo(&st_before) == 0, "getpinfo");
	      printf(1, "\n ****PInfo before**** \n");
	      print(&st_before);
        printf(1,"Spinning...\n");
	
        spin();
	
        check(getpinfo(&st_after) == 0, "getpinfo");
	      printf(1, "\n ****PInfo after**** \n");
	      print(&st_after);
	
        compare(pid_low, pid_high, &st_before, &st_after);
	      printf(1, "Should print 1"); 
	      exit();
   }
   spin();
   printf(1, " then 2");
   while (wait() > -1);
   exit();
}
