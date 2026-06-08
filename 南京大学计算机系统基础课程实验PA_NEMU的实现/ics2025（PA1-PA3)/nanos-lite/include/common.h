#ifndef __COMMON_H__
#define __COMMON_H__

/* Uncomment these macros to enable corresponding functionality. */
#define HAS_CTE
//#define HAS_VME
//#define MULTIPROGRAM
//#define TIME_SHARING

#define ENTRY_BIN "/bin/nterm"



#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <debug.h>


Context* schedule(Context *prev);


#endif
