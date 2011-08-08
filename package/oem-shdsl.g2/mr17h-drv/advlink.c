#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/config.h>
#include <asm/types.h>

#include "advlink.h"
#include "sg17debug.h"

void 
advlink_init(struct advlink_t *alink)
{
    memset(alink,0,sizeof(*alink)); 
}

void
advlink_enable(struct advlink_t *alink)
{
    alink->enabled = 1;
}

void 
advlink_disable(struct advlink_t *alink)
{
    alink->enabled = 0;
}

int
advlink_enabled(struct advlink_t *alink)
{
    return alink->enabled;
}


void 
advlink_hwstatus(struct advlink_t *alink,int hstat)
{
    alink->hw_link = hstat;
}

int
advlink_get_hwstatus(struct advlink_t *alink)
{
    return alink->hw_link;
}

int
advlink_send(struct advlink_t *alink,u32 *buf)
{
    int i;
    char *ptr = (char *)buf;
    for(i=0;i<sizeof(u32);i++){
	ptr[i] = i+1;
    }
    buf[1] = ++alink->send_cntr;
    buf[2] = alink->rem_cntr;

    PDEBUG(debug_link,"send_cntr=%d,recv_cntr=%d",alink->send_cntr,alink->recv_cntr);

    return 0;
}

void
advlink_send_error(struct advlink_t *alink)
{
    alink->send_cntr--;
}


int
advlink_recv(struct advlink_t *alink,u32 *buf)
{
    int i;
    char *ptr = (char *)buf;
    for(i=0;i<sizeof(u32);i++){
	if( ptr[i] != i+1 ){
	    PDEBUG(debug_link,"magic sequence error");
	    return -1;
	}
    }
    alink->rem_cntr = buf[1];
    alink->recv_cntr = buf[2];
    PDEBUG(debug_link,"rem_cntr=%d,recv_cntr=%d",alink->rem_cntr,alink->recv_cntr);
    return 0;
}

int 
advlink_status(struct advlink_t *alink)
{
    if( !alink->enabled ){
	return alink->hw_link;
    }
    
    if( alink->hw_link == ADVLINK_UP 
	&& (alink->send_cntr - alink->recv_cntr) < 4 ){
	return ADVLINK_UP;
    }else{
	return ADVLINK_DOWN;
    }
    return ADVLINK_DOWN;
}
