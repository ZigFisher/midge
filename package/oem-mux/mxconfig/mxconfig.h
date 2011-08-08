#ifndef MXCONFIG_H
#define MXCONFIG_H

#include "dev_interface.h"
#include "mxobjects.h"
#include "ltypes.h"

#define mxerror(fmt,args...) printf("ERROR: " fmt "\n",  ##args)
#define mxwarn(fmt,args...) printf("WARNING: " fmt "\n",  ##args)

#endif
