#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#define check(exp, msg) if(exp) {} else {\
  printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
  exit();}
#define tol 55

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

int checkVariance(int *pid_chds, struct pstat *before, struct pstat *after, int numprocs)
{
  // printf(1,"check var starts:\n numprocs:%d\n",numprocs);
  printf(1,"something is wrong with this test case, print diffs[i] to calculate manually!!!!!\n");
  int count = 0;
  int sum = 0;
  int diffs[numprocs];
  int i;
  for (i = 0; i < numprocs; i++){
    int pid = pid_chds[i];
       
    // printf(1,"pid:%d,",pid);
       
    if (pid == -1)
      break;
    diffs[i] = 0;
    int j;
    for (j = 0; j < numprocs; j++){
      if (before->pid[j] == pid){
	int k;
	for (k = 0; k < numprocs; k++){
	  if (after->pid[k] == pid){

	    // printf(1,"matched pid:%d,",pid);

	    diffs[i] = after->ticks[k] - before->ticks[j];
	    
	    printf(1,"diffs[%d]: %d\n",i,diffs[i]);
	   
	    sum += diffs[i];
	    count++;
	    break;
	  }
	}
	break;
      }
    }
  }
    
   printf(1,"sum:%d\n",sum);
   printf(1,"count:%d\n",count);
   
  if (count < numprocs/2){
    printf(1,"check failed: Too few processes lasted\n");
    return -1;
  }
  if (sum < count+1){
    printf(1,"check failed: Child processes didn't run enough\n");
    return -1;
  }
   
  double avg = (double)sum / count;
  double var = 0;
  sum = 0;
  for (i = 0; i < numprocs; i++){
    if (pid_chds[i] != -1){
      var += (avg - diffs[i]) * (avg - diffs[i]);
      if (diffs[i] == 0)
	sum++;
    }
    else
      break;
  }
  if (sum * 2 > numprocs){
    printf(1,"check failed: Too many children didn't run\n");
    return -1;
  }
  var /= count;
  //printf(1, "Variance: ~%d\n",(int)var);
  //  printf(1, "Variance: %d\n",var);
  if (var < tol)
  return 0;
  else{
    printf(1,"check failed: Variance too high\n");
    return -1;
  }
}

int
main(int argc, char *argv[])
{
  int numtickets = 5371;
  int pid_chds[NPROC];
  int i = 0;
  check(settickets(numtickets) == 0, "settickets");
  printf(1,"Goal: Achieve variance of <%d in ticks for many processes with the same number of tickets\n",tol);
   
  for (; (pid_chds[i] = fork()) >= 0; i++){
    if (pid_chds[i] == 0){
      for (;;){
	spin();
      }
    }
  }
  if (settickets(numtickets*3+1) != 0){
    printf(1,"check failed: settickets\n");
    goto Cleanup;
  }
   
  struct pstat st_before, st_after;
  
  if (getpinfo(&st_before) != 0){
    printf(1,"check failed: getpinfo\n");
    goto Cleanup;
  }
  printf(1, "\n ****PInfo before**** \n");
  print(&st_before);
  printf(1, "Spinning...");
  sleep(1);
  int j;
  for (j = i; j < NPROC; j++){
    pid_chds[j] = -1;
  }
  spin();
    
  if (getpinfo(&st_after) != 0){
    printf(1,"check failed: getpinfo\n");
    goto Cleanup;
  }
   
  if (settickets(10000000) != 0){
    printf(1,"check failed: settickets\n");
    goto Cleanup;
  }
   
  printf(1, "\n ****PInfo after**** \n");
  print(&st_after);
   
  if (checkVariance(pid_chds,&st_before,&st_after,i) < 0)
    goto Cleanup;
  printf(1, "Variance check passed. Within limits. \n");
  printf(1, "Should print 1 then 2");
   
 Cleanup:
  for (i = 0; pid_chds[i] > 0; i++){
    kill(pid_chds[i]);
  }
  while(wait() > -1);

  exit();
}
