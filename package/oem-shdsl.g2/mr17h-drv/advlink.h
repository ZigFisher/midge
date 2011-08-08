#ifndef ADVANCED_LINK_CHECK
#define ADVANCED_LINK_CHECK

#include <asm/types.h>

#define ADVLINK_MSIZE32 3
#define ADVLINK_MSIZE8 4*ADVLINK_MSIZE32
#define ADVLINK_UP 1
#define ADVLINK_DOWN 0

struct advlink_t{
    u32 send_cntr,recv_cntr,rem_cntr;
    u8 hw_link:1;
    u8 virt_link:1;
    u8 enabled:1;
};

void advlink_init(struct advlink_t *alink);
void advlink_enable(struct advlink_t *alink);
void advlink_disable(struct advlink_t *alink);
int advlink_enabled(struct advlink_t *alink);
void advlink_hwstatus(struct advlink_t *alink,int hstat);
int advlink_get_hwstatus(struct advlink_t *alink);
int advlink_send(struct advlink_t *alink,u32 *buf);
void advlink_send_error(struct advlink_t *alink);
int advlink_recv(struct advlink_t *alink,u32 *buf);
int advlink_status(struct advlink_t *alink);
	    
#endif
