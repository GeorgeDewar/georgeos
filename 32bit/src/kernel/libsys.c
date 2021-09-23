#include "libsys.h"

DEFN_SYSCALL1(print, 0, const char *);
DEFN_SYSCALL3(get_string, 1, char *, int, int);
