#ifndef SIMULATOR_CUSTOMASSERT_H
#define SIMULATOR_CUSTOMASSERT_H

#ifdef NDEBUG
#define assert(condition, message) ((void)0)
#else
#include <stdio.h>

#define assert(condition, message)              \
    do {                                        \
	    if(__builtin_expect (condition, true)); \
		else {                                  \
	        printf("FATAL: ");                  \
	        printf(message);                    \
            fflush(stdout);                     \
	        abort();                            \
	    }                                       \
    } while(false)

#endif

#endif //SIMULATOR_CUSTOMASSERT_H
