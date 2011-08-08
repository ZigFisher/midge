#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "mxobjects.h"
#include "debug.h"

//------------ MX line ----------------//

mxline_t *
mxline_init()
{
    mxline_t *new = malloc(sizeof(*new));
    if( !new )
	return NULL;
    memset(new,0,sizeof(*new));
    return new;
}

int
mxline_add(mxline_t *l,domain_t domain,mxelem_t el,int ind)
{
	PDEBUG(DINFO,"line%d devcnt=%d, domain=%d,domain=%d",ind,l->devcnt,l->domain,domain);
    if( !(l->devcnt < MAX_IFS) )
	    return -ELSPACE;
    if( !l->devcnt ){
    	l->domain = domain;
    }else if( l->domain != domain ){
	    l->domain_err = 1;
    }

    l->devs[l->devcnt] = el;
    l->devcnt++;
    return 0;
}

void
mxline_free(mxline_t *l)
{
    l->devcnt = 0;
}

