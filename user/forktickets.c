#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#define check(exp, msg) if(exp) {} else {\
	printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
	exit();}

int
main(int argc, char *argv[]){
	int pid_par = getpid();
	int tickets = 5;
	check(settickets(tickets) == 0, "settickets");

	if(fork() == 0){
		int pid_chd = getpid();
		struct pstat st;
	
		check(getpinfo(&st) == 0, "getpinfo");
		int tickets_par = -1,tickets_chd = -1;
                int i;
		for(i = 0; i < NPROC; i++){
      if (st.pid[i] == pid_par){
				tickets_par = st.tickets[i];
			}
			else if (st.pid[i] == pid_chd){
				tickets_chd = st.tickets[i];
			}
		}
		check(tickets_par == tickets, "Tickets number in pinfo should be correct");
		printf(1, "parent: %d, child: %d\n", tickets_par, tickets_chd);
		check(tickets_chd == tickets, "Expected the number of tickets in the child process to equal the number of tickets in the parent process");
    printf(1, "Should print 1 then 2");
    exit();
	}
  while (wait() > 0);
	exit();
}
