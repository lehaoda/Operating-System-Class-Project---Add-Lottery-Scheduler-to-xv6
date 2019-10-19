#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

#define check(exp, msg) if(exp) {} else {\
   printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
   exit();}

int
main(int argc, char *argv[])
{
   struct pstat st;
   int pid = getpid();
   int defaulttickets = 0;
   check(getpinfo(&st) == 0, "getpinfo");

   printf(1, "\n **** PInfo **** \n");
   int i;
   for(i = 0; i < NPROC; i++) {
      if (st.inuse[i]) {
        if(st.pid[i] == pid) {
          defaulttickets = st.tickets[i];
          printf(1, "pid: %d tickets: %d ticks: %d\n", st.pid[i], st.tickets[i], st.ticks[i]);
         }
      }
   }

   check(defaulttickets == 1, "The default number of tickets for each process should be 1");
   printf(1, "Should print 1 then 2");
   exit();
}
