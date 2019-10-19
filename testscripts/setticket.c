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
   check(settickets(1) == 0, "settickets to 1");
   check(settickets(1000000) == 0, "settickets to 1000000");
   check(settickets(0) == -1, "settickets to 0");
   check(settickets(-1) == -1, "settickets to < 0");
   printf(1, "Should print 1 then 2");
   exit();
}
