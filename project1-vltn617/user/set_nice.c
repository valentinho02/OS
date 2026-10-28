#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
    set_nice(atoi(argv[1]));
    exit(0);
}
