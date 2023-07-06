#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
mysleep(int n)
{
    sleep(n);
}

int
main(int argc, char *argv[])
{
  if(argc <= 1){
    exit(1);
  }

  mysleep(atoi(argv[2]));
  exit(0);
}
