#ifndef HMO_VERSION_INCLUDED
#define HMO_VERSION_INCLUDED

#include "revision.h"

#define HMO_STR(_s) #_s
#define HMO_XSTR(_s) HMO_STR(_s)
#define HMO_VERSION "1.0.0 " HMO_XSTR(HMO_REVISION)

#endif

