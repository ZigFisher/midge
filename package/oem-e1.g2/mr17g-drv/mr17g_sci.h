/* mr17g_sci.h
 *  Sigrand MR17G E1 PCI adapter driver for linux (kernel 2.6.x)
 *
 *	Written 2008 by Artem Y. Polyakov (artpol84@gmail.com)
 *
 *	This driver presents MR17G modem to OS as common hdlc interface.
 *
 *	This software may be used and distributed according to the terms
 *	of the GNU General Public License.
 *
 */
#ifndef MR17G_SCI_H
#define MR17G_SCI_H

#include "mr17g.h"

int mr17g_sci_enable(struct mr17g_card *card);
int mr17g_sci_disable(struct mr17g_card *card);
void mr17g_sci_endmon(struct mr17g_card *card);
irqreturn_t mr17g_sci_intr(int irq,void *dev_id,struct pt_regs *regs);
int mr17g_sci_request(struct mr17g_sci *sci,int cnum,char buf[SCI_BUF_SIZE],int size,int acksize);
void pef22554_defcfg(struct mr17g_channel *chan);

void mr17g_sci_monitor(void *data);

//debug
void mr17g_sci_dump(u8 *addr,int size);
void mr17g_sci_memcheck(u8 *addr);
void mr17g_sci_memcheck1(u8 *addr);


#endif
