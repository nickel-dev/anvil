#include "base.h"
#include "math.h"

#if OS_WIN32
#include "os/core_win32.c"
#endif

#if OS_LINUX
#include "os/core_linux.c"
#endif