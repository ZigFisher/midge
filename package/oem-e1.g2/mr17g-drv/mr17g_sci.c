/* mr17g_sci.c
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

#include "mr17g.h"
#include "mr17g_sci.h"
#include "mr17g_net.h"
#include "pef22554.h"
// Debug settings
#define DEBUG_ON
#define DEFAULT_LEV 0
#include "mr17g_debug.h"

int
mr17g_sci_enable(struct mr17g_card *card)
{
	struct mr17g_sci *sci = &card->sci;
	volatile struct mr17g_sci_regs *regs = &sci->iomem->regs;
	
    // setup HDLC registers
	iowrite8(0,&regs->CRA);
	mdelay(100);
	iowrite8((XRST0|XRST1|RXEN),&regs->CRA);
	iowrite8(1,&regs->SR);
    iowrite8((RXS | TXS | CRC | COL | OFL),&regs->IMR);	

    // Init chip locking
	init_MUTEX(&sci->sem);

    // Init interrupt wait queue
    init_waitqueue_head( &sci->wait_q );
	// Initialize work queue for link monitoring
	INIT_WORK(&sci->wqueue, mr17g_sci_monitor,(void*)card);
    // setup SCI controller
    if( pef22554_setup_sci(card) ){
        goto error;
    }
    // Display result HDLC registers content
    PDEBUG(debug_sci,"CRA=%02x, CRB=%02x, IMR=%02x, SR=%02x",
    	ioread8(&regs->CRA),ioread8(&regs->CRB),
    	ioread8(&regs->IMR),ioread8(&regs->SR));
	PDEBUG(debug_sci,"SCI enabled");
	return 0;
error:
	iowrite8( 0,&regs->CRA);
    iowrite8(0,&regs->IMR);	
    return -1;
}

int
mr17g_sci_disable(struct mr17g_card *card)
{
	struct mr17g_sci *sci = &card->sci;
	volatile struct mr17g_sci_regs *regs = &sci->iomem->regs;

	PDEBUG(debug_sci,"start");
	// shut down device
	iowrite8(0,&regs->CRA);
	iowrite8(0,&regs->IMR);	
	iowrite8(0xff,&regs->SR);
	PDEBUG(debug_sci,"SCI disabled");
	return 0;
}

void
mr17g_sci_endmon(struct mr17g_card *card)
{
	struct mr17g_sci *sci = &card->sci;
	PDEBUG(debug_sci,"start");
    // Cancel monitoring
	cancel_delayed_work(&sci->wqueue);
}

int 
mr17g_sci_request_one(struct mr17g_sci *sci,int chipnum,char buf[SCI_BUF_SIZE],int size)
{
    volatile struct mr17g_sci_iomem *mem = sci->iomem;
	volatile struct mr17g_sci_regs *regs = &mem->regs;
	u8 tmp = ioread8(&regs->CRA);
    int i,ret;

	PDEBUG(debug_sci,"start, CRA=%02x, CRB=%02x, IMR=%02x, SR=%02x",
		ioread8(&regs->CRA),ioread8(&regs->CRB),ioread8(&regs->IMR),ioread8(&regs->SR));	

    // SCI is busy now, fail to transmit

    //---------------- Transmit message --------------------

	
    if( tmp & TXEN ){
        printk(KERN_ERR"%s: SCI transmitter already in use\n",MR17G_MODNAME);
        size = -EAGAIN;
		goto exit;
    }
    // Check size of buffer
    if( size > SCI_BUF_SIZE ){
        printk(KERN_ERR"%s: bad message size(%d) in SCI xmit function\n",MR17G_MODNAME,size);
        size = -EINVAL;
		goto exit;
    }
    PDEBUG(debug_sci,"Fill SCI buffer");
	

    // Move outgoing data to toransmit buffer
	for( i=0; i<size; i++)
		iowrite8( buf[i],(u8*)mem->tx_buf + i);

    // Prepare for transmission
    sci->rxs = 0;
    sci->crc_err = 0;
	iowrite16(0,&regs->RXLEN);
	iowrite16(size,&regs->TXLEN);
	// Chip select
	if( !chipnum ){
		u8 cra = ioread8(&regs->CRA) & (~CSEL);
		iowrite8(cra,&regs->CRA);
	}else{
		u8 cra = ioread8(&regs->CRA) | CSEL;
		iowrite8(cra,&regs->CRA);
	}
    // Delay for hardware correct work
    mdelay(3);

//    PDEBUG(debug_sci,"Before enable transmitter. First part of message, second - registers");
//    mr17g_sci_dump((u8*)&sci->tx_buf,7);
//    mr17g_sci_dump((u8*)&sci->regs,7);

	PDEBUG(debug_sci,"start, CRA=%02x, CRB=%02x, IMR=%02x, SR=%02x",
		ioread8(&regs->CRA),ioread8(&regs->CRB),ioread8(&regs->IMR),ioread8(&regs->SR));	


    // Enable SCI transmitter
	iowrite8((ioread8(&regs->CRA)|TXEN ),&regs->CRA);

    //----------------- Receive reply ------------------------
    // Wait for message
    if( !sci->rxs ){
       ret = interruptible_sleep_on_timeout( &sci->wait_q,HZ/100);
    }else{
        ret = 1;
    }
    PDEBUG(debug_sci,"After interrupt wait");	
    // Check correctness of transmission and receiving
	if( sci->crc_err || !sci->rxs ){
		PDEBUG(debug_error,"Collision detected, sci.rxs = %d, sci.crc_err=%d",sci->rxs,sci->crc_err);
		sci->crc_err = 0;
        size = -1;
        goto exit;
	}
	
    // Read & check incoming message length
	size = ioread16(&regs->RXLEN);
    PDEBUG(debug_sci,"size = %d", size);
	if( !size ){
		PDEBUG(debug_error,"Zero length");
		size = -EAGAIN; 
        goto exit;
	}else if(size > SCI_BUF_SIZE) {
        printk(KERN_ERR"%s: in SCI recv incoming size(%d) > MAX\n",MR17G_MODNAME,size);
		size = -EINVAL; 
        goto exit;
	}

    // Move incoming message to local buffer
	for( i=0; i<size; i++){
		buf[i] = ioread8((u8*)mem->rx_buf + i);
    }

//----------- DEBUG ---------------------------
//    PDEBUGL(debug_sci,"Recv msg:");
//    for( i=0; i<size; i++){
//       PDEBUGL(debug_sci,"%02x ",buf[i] & 0xff);
//    }
//    PDEBUGL(debug_sci,"\n");
//----------- DEBUG ---------------------------

exit:
    // Restore receiver on error exit
	iowrite8((ioread8(&regs->CRA)|RXEN),&regs->CRA);		
	PDEBUG(debug_sci,"end");	
    return size;
}


int 
mr17g_sci_request(struct mr17g_sci *sci,int cnum,char buf[SCI_BUF_SIZE],int size,
		int acksize)
{
    int i,ret = 0;
	
    // Lock chipset until end of request
    if( down_interruptible(&sci->sem) )
		return -EAGAIN;
	
	// Try to make request several times
    for(i=0;i<10;i++){
        if( (ret = mr17g_sci_request_one(sci,cnum,buf,size)) == acksize ){
            goto exit;
        }
        PDEBUG(debug_error,"Iter %d, error: ret=%d",i,ret);
    }
    if( ret > 0 )
    	ret *= -1;
exit:
    // Unlock chip
    up(&sci->sem);
    return ret;
}


irqreturn_t
mr17g_sci_intr(int  irq,  void  *dev_id,  struct pt_regs  *ptregs )
{
	struct mr17g_sci *sci = (struct mr17g_sci *)dev_id;
	volatile struct mr17g_sci_regs *regs = &sci->iomem->regs;
	u8 mask = ioread8(&regs->IMR);
	u8 status = (ioread8(&regs->SR) & mask);	

	PDEBUG(debug_sci,"start");

	if( !status )
		return IRQ_NONE;

	iowrite8(0xff,&regs->SR);   // ack all interrupts
	iowrite8(0,&regs->IMR);		// disable interrupts

	if( status & TXS ){
		PDEBUG(debug_sci,"TXS");
		sci->tx_packets++;
		sci->tx_bytes += ioread16(&regs->TXLEN);
	}

	if( status & RXS ){
		int in_len;
		PDEBUG(debug_sci,"RXS");
		in_len = ioread16(&regs->RXLEN);
		sci->rx_packets++;
		sci->rx_bytes += in_len;
		wake_up(&sci->wait_q );
        sci->rxs = 1;
	}

	if( status & CRC ){
        sci->crc_errors++;
		sci->crc_err = 1;
		iowrite8( (ioread8(&regs->CRA )|RXEN),&regs->CRA );
		PDEBUG(debug_error,"CRC");
		wake_up(&sci->wait_q );
	}
	
	if( status & COL ){
        sci->tx_collisions++;
		PDEBUG(debug_error,"COL");
		wake_up( &sci->wait_q );
	}
	
	iowrite8(mask, &regs->IMR); // enable interrupts
	return IRQ_HANDLED;
}

void
mr17g_sci_monitor(void *data)
{
	struct mr17g_card *card = (struct mr17g_card *)data;
    int i,j;
    for(i=0;i<card->chip_quan;i++){
    	struct mr17g_chip *chip = card->chips + i;
    	for(j=0;j<chip->if_quan;j++){
        	mr17g_net_link(chip->ifs[j]);
    	}
    }
    // Schedule next monitoring
	schedule_delayed_work(&card->sci.wqueue,2*HZ);
}


void
mr17g_sci_dump(u8 *addr, int size)
{
    int i;
    PDEBUGL(debug_sci,"I/O Memory window (from %p):",addr);
    for(i=0;i<size;i++){
        if(!(i%32)){
            PDEBUGL(debug_sci,"\n0x%04x-0x%04x: ",i,i+32);
        }
        PDEBUGL(debug_sci,"%02x ",addr[i]);
    }
    PDEBUGL(debug_sci,"\n");
}












// ------------------------ OLD debugging stuff -----------------------//
/*
void 
mr17g_sci_memcheck(u8 *addr){
    int i,j;
    int flag = 0;
return;
    for(i=0;i<4;i++){
        u8 tmp = 1;
        // Zero check
        *(addr + i) = 0;
        if( *(addr + i) ){
            PDEBUG(debug_sci,"byte %d, zero check error",i);
            flag = 1;
        }   
        for(j=0;j<8;j++,tmp*=2){
            *(addr + i) = tmp;
//            PDEBUG(debug_sci,"byte %d, val = %d, actual = %d",i,tmp,*(addr+i));
            if( *(addr + i) != tmp ){
                PDEBUG(debug_sci,"byte %d, val = %d, actual = %d",i,tmp,*(addr+i));
                flag = 1;
            }
        }
    }
    if( !flag ){
        PDEBUG(debug_sci,"Memcheck - SUCCESS");
    }
}


void 
mr17g_sci_memcheck1(u8 *addr){
    int i,j;
    int flag = 0;
return;
    for(i=0;i<4;i++){
        u8 tmp = 1;
        // Zero check
        iowrite8(0,addr + i);
        if( ioread8(addr + i) ){
            PDEBUG(debug_sci,"byte %d, zero check error",i);
            flag = 1;
        }   
        for(j=0;j<8;j++,tmp*=2){
            iowrite8(tmp,addr + i);
//            PDEBUG(debug_sci,"byte %d, val = %d, actual = %d",i,tmp,ioread8(addr+i));
            if( ioread8(addr + i) != tmp ){
                PDEBUG(debug_sci,"byte %d, val = %d, actual = %d",i,tmp,ioread8(addr+i));
                flag = 1;
            }
        }
    }
    if( !flag ){
        PDEBUG(debug_sci,"Memcheck - SUCCESS");
    }
}

*/
