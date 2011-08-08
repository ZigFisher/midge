#ifndef MXOBJECTS_H
#define MXOBJECTS_H

#include "dev_interface.h"
#include "ltypes.h"

#define MAX_IFS 100
#define MX_LINES 16
#define MX_LWIDTH 256
#define MX_LWIDTH32 8


// Errors
#define ELSPACE 1
#define EDOMAIN 2


typedef enum { clkA, clkB } domain_t;

typedef struct{
    ifdescr_t *ifd;
    u8 xfs;
    u8 mxrate;
} mxelem_t;

typedef struct{
    domain_t domain;
    u8 domain_err;
    mxelem_t devs[MAX_IFS];
    int devcnt;
} mxline_t;

typedef struct{
    mxline_t *a[MX_LINES];
    int lnum;
} mxdomain_t;

mxline_t *mxline_init();
int mxline_add(mxline_t *l,domain_t domain,mxelem_t el,int ind);
void mxline_free(mxline_t *l);

#endif

