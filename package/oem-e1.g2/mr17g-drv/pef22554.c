/* pef22554.c
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
#include "pef22554.h"

#define DEBUG_ON
#define DEBUG_LEV 10
#include "mr17g_debug.h"


// Chiset register access
int 
pef22554_setup_sci(struct mr17g_card *card)
{
	struct mr17g_sci *sci = &card->sci;
    char buf[SCI_BUF_SIZE];
	struct pef22554_write_cmd *cmd = (struct pef22554_write_cmd*)buf;
    struct pef22554_write_ack *ack;
    int size,i;

    // Setup PEF22554 SCI controller for all chips
    for(i=0;i<card->chip_quan;i++){
    	PDEBUG(0,"Set SCI for chip #%d",i);
	    memset((u8*)cmd,0,sizeof(*cmd));
    	cmd->src.addr = 0x3f;
    	cmd->src.cr = 0;
	    cmd->dst.addr = i; 
    	cmd->dst.cr = 1;
    	cmd->reg_addr = 0; // First register
    	cmd->ctrl_bits = WR_SCI; 
    	cmd->data = (PP | ACK_EN | CRC_EN | DUP ); //0x95;
    	   
    	if((size=mr17g_sci_request(sci,i,buf,PEF22554_WCMD_SIZE,PEF22554_WACK_SIZE)) < 0){
        	printk(KERN_ERR"%s: Error setting SCI controller, size=%d, need_size = %d\n",
				MR17G_MODNAME,size,PEF22554_WACK_SIZE);
        	goto error;
    	}
    	
    	ack = (struct pef22554_write_ack*)buf;
    	if( !( ack->rsta.vfr && ack->rsta.crc && !ack->rsta.rab ) ){
        	printk(KERN_ERR"%s: Error setting SCI controller, RSTA=%02x\n",
            	    MR17G_MODNAME,*((u8*)&ack->rsta));
        	goto error;
    	}
    }
    return 0;
error:
    return -1;
}

void
pef22554_defcfg(struct mr17g_channel *chan)
{   
    struct mr17g_chan_config *cfg = &chan->cfg;
    memset(cfg,0,sizeof(*cfg));
    // Default config: framed, HDB3, slotmap = 1-31;
    cfg->framed = 1;
    cfg->hdb3 = 1;
    cfg->slotmap = 0xFFFFFFFE;
    cfg->crc16 = 1;
    cfg->fill_7e = 1;
	cfg->ts16 = 1;
}

int
pef22554_writereg(struct mr17g_chip *chip,u8 chan,u16 addr,u8 val)
{
	struct mr17g_sci *sci = chip->sci;
    char buf[SCI_BUF_SIZE];
    struct pef22554_write_cmd *cmd = (struct pef22554_write_cmd*)buf;
    struct pef22554_write_ack *ack;
    int size;

    PDEBUG(debug_pef,"Chan%d SetReg(%04x) = %02x",chan,addr,val);
    
    memset(cmd,0,sizeof(*cmd));
    cmd->src.addr = 0x3f;
    cmd->src.cr = 1;
    cmd->dst.addr = 0; 
    cmd->dst.cr = 1;
    cmd->reg_addr = addr;
    if( !PEF22554_IS_GENERAL(addr) ){
        cmd->reg_addr |= (chan << 8);
    }
    cmd->ctrl_bits = WR_FALC;
    cmd->data = val;
    if( (size = mr17g_sci_request(sci,chip->num,buf,PEF22554_WCMD_SIZE,
    			PEF22554_WACK_SIZE)) <  0){
        printk(KERN_ERR"%s: Error setting QuadFALC register %04x = %02x, size=%d(need %d)\n",
                MR17G_MODNAME,addr,val,(-1)*size,PEF22554_WACK_SIZE);
        goto error;
    }
    ack = (struct pef22554_write_ack*)buf;
    if( !( ack->rsta.vfr && ack->rsta.crc && !ack->rsta.rab ) ){
        printk(KERN_ERR"%s: Error setting QuadFALC register %04x = %02x, RSTA=%02x\n",
                MR17G_MODNAME,addr,val,*((u8*)&ack->rsta));
        goto error;
    }
    return 0;
error:
    return -1;
}    

int
pef22554_readreg(struct mr17g_chip *chip,u8 chan,u16 addr,u8 *val)
{
	struct mr17g_sci *sci = chip->sci;
    char buf[SCI_BUF_SIZE];
    struct pef22554_read_cmd *cmd = (struct pef22554_read_cmd*)buf;
    struct pef22554_read_ack *ack;
    int size;

    PDEBUG(debug_pef,"Chan%d ReadReg(%04x)",chan,addr);
    
    memset(cmd,0,sizeof(*cmd));
    cmd->src.addr = 0x3f;
    cmd->src.cr = 1;
    cmd->dst.addr = 0; 
    cmd->dst.cr = 1;
    cmd->reg_addr = addr;
    if( !PEF22554_IS_GENERAL(addr) ){
        PDEBUG(debug_pef,"Channel specific reg %04x",addr);
        cmd->reg_addr |= (chan << 8);
    }
    cmd->ctrl_bits = RD_FALC;
    cmd->rdepth = 1;
    if( (size = mr17g_sci_request(sci,chip->num,buf,PEF22554_RCMD_SIZE,
    			PEF22554_RACK_SIZE))<0 ){
        printk(KERN_ERR"%s: Error reading QuadFALC register %04x, size=%d(need %d)\n",
                MR17G_MODNAME,addr,(-1)*size,PEF22554_RACK_SIZE);
        goto error;
    }
    ack = (struct pef22554_read_ack*)buf;
    if( !( ack->rsta.vfr && ack->rsta.crc && !ack->rsta.rab ) ){
        printk(KERN_ERR"%s: Error reading QuadFALC register %04x, RSTA=%02x\n",
                MR17G_MODNAME,addr,*((u8*)&ack->rsta));
        goto error;
    }
    *val = ack->reg_cont;
    return 0;
error:
    return -1;
}    


// Basic Sigrand chipset setup
int
pef22554_basic_chip(struct mr17g_chip *chip)
{
    if( pef22554_writereg(chip,0,IPC,0x01) ){
        goto error;
    }
    if( pef22554_writereg(chip,0,GPC1,0x00) ){
        goto error;
    }
    if( pef22554_writereg(chip,0,GPC2,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,0,GCM1,0x00)){
        goto error;
    }  // 16.384 MHz MCLK
    if( pef22554_writereg(chip,0,GCM2,0x18)){
        goto error;
    }
    if( pef22554_writereg(chip,0,GCM3,0xFB)){
        goto error;
    }
    if( pef22554_writereg(chip,0,GCM4,0x0B)){
        goto error;
    }
    if( pef22554_writereg(chip,0,GCM5,0x01)){
        goto error;
    }
    if( pef22554_writereg(chip,0,GCM6,0x0B)){
        goto error;
    }
    if( pef22554_writereg(chip,0,GCM7,0xDB)){
        goto error;
    }
    if( pef22554_writereg(chip,0,GCM8,0xDF)){
        goto error;
    }
    if( pef22554_writereg(chip,0,GIMR,0x01)){
        goto error;
    }  // masked
    if( pef22554_writereg(chip,0,GPC3,0x21)){
        goto error;
    }
    if( pef22554_writereg(chip,0,GPC4,0x03)){
        goto error;
    }
    if( pef22554_writereg(chip,0,GPC5,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,0,GPC6,0x20)){
        goto error;
    }
    if( pef22554_writereg(chip,0,INBLDTR,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,0,PRBSTS1,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,0,PRBSTS2,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,0,PRBSTS3,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,0,PRBSTS4,0x00)){
        goto error;
    }
    return 0; 
error:
    return -1;
}


int
pef22554_basic_channel(struct mr17g_channel *chan)
{
    struct mr17g_chip *chip = chan->chip;
    int ch = chan->num;

    if( pef22554_writereg(chip,ch,CCR1,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,CCR2,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,RDICR,0x00)){   // RA/RDI - 1 MF
        goto error;
    }
    if( pef22554_writereg(chip,ch,RTR1,0x00)){  // HDLC1 - not used 
        goto error;
    }
    if( pef22554_writereg(chip,ch,RTR2,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,RTR3,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,RTR4,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,TTR1,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,TTR2,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,TTR3,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,TTR4,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,IMR0,0xFF)){  // all masked
        goto error;
    }
    if( pef22554_writereg(chip,ch,IMR1,0xFF)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,IMR2,0xFF)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,IMR3,0xFF)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,IMR4,0xFF)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,IMR5,0xFF)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,IMR6,0xFF)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,LOOP,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XSW,0x9F)){  // all Sa, Si = 1
        goto error;
    }
    if( pef22554_writereg(chip,ch,XC0,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XC1,0x04)){  // tx offset = 0
        goto error;
    }
    if( pef22554_writereg(chip,ch,RC0,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,RC1,0x04)){  // rx offset = 0 
        goto error;
    }
    if( pef22554_writereg(chip,ch,XPM0,0x9C)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XPM1,0x03)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XPM2,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,TSWM,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,SIC4,0x06)){ // SYPR Edge
        goto error;
    }
    if( pef22554_writereg(chip,ch,IDLE,0xFF)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XSA4,0xFF)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XSA5,0xFF)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XSA6,0xFF)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XSA7,0xFF)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XSA8,0xFF)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,FMR3,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,ICB1,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,ICB2,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,ICB3,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,ICB4,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,LIM0,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,PCD,0x0F)){   // 256 '0's LOS alarm
        goto error;
    }
    if( pef22554_writereg(chip,ch,PCR,0x1F)){   // 32 '1's LOS clear 
        goto error;
    }
    if( pef22554_writereg(chip,ch,LIM2,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,LCR1,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,LCR2,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,LCR3,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,SIC1,0xCA)){  // 16.384 MHz
        goto error;
    }
    if( pef22554_writereg(chip,ch,SIC2,0x00)){  // channel phase 0
        goto error;
    }
    if( pef22554_writereg(chip,ch,SIC3,0x04)){  // EDGE
        goto error;
    }
    if( pef22554_writereg(chip,ch,CMR4,0x05)){  // 16.384 MHz
        goto error;
    }
//    printk(KERN_NOTICE"Chan %d: WRITE to CMR5 %02x\n",ch,ch<<5);
    if( pef22554_writereg(chip,ch,CMR5,ch<<5)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,CMR6,0x0C)){  // 16.384 MHz
        goto error;
    }
    if( pef22554_writereg(chip,ch,CMR1,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,CMR2,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,GCR,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,ESM,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,CMR3,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,DEC,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XS1,0x0B)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XS2,0xDD)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XS3,0xDD)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XS4,0xDD)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XS5,0xDD)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XS6,0xDD)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XS7,0xDD)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XS8,0xDD)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XS9,0xDD)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XS10,0xDD)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XS11,0xDD)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XS12,0xDD)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XS13,0xDD)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XS14,0xDD)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XS15,0xDD)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,XS16,0xDD)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,PC1,0x00)){  // SYPR, SYPX
        goto error;
    }
    if( pef22554_writereg(chip,ch,PC2,0xBB)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,PC3,0xBB)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,PC4,0xBB)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,PC6,0x00)){  // 2 Ohm
        goto error;
    }
    if( pef22554_writereg(chip,ch,BFR,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,ALS,0x00)){
        goto error;
    }
    if( pef22554_writereg(chip,ch,IMR7,0x18)){  // masked
        goto error;
    }
    if( pef22554_writereg(chip,ch,WCON,0x03)){
        goto error;
    }

    return 0;
error:
    return -1;
}

int
pef22554_channel(struct mr17g_channel *chan)
{   
    struct mr17g_chip *chip = chan->chip;
    struct mr17g_chan_config *cfg = &chan->cfg;
    volatile struct mr17g_hw_regs *regs = chan->iomem.regs;
    u8 tmp;

	// Check inter-option depends
	if( !cfg->framed ){
		cfg->cas=0;
		cfg->crc4=0;
	}else if( cfg->ts16 ){
		cfg->cas = 0;
	}

    // FMR0: 0xF0 - HDB3, 0xA0 - AMI
    switch( cfg->hdb3 ){
    case 0: tmp = 0xA0; break;
    case 1: tmp = 0xF0; break;
    default:
        printk(KERN_ERR"%s: Error: unexpecter hdb3 value = %d\n",MR17G_MODNAME,cfg->hdb3);
        goto error;
    }
    if( pef22554_writereg(chip,chan->num,FMR0,tmp)){
        goto error;
    }

    // FMR1: 0x42 - no CRC4, 0x4A - CRC4
    switch( cfg->crc4 ){
    case 0: tmp = 0x42; break;
    case 1: tmp = 0x4A; break;
    default:
        printk(KERN_ERR"%s: Error: unexpecter crc4 value = %d\n",MR17G_MODNAME,cfg->crc4);
        goto error;
    }
    if( pef22554_writereg(chip,chan->num,FMR1,tmp)){
        goto error;
    }

    // FMR2: 0x30 - unframed, 0x00 - no CRC4, 0x81 - CRC4 
    if( !cfg->framed ){
        tmp = 0x30;
    }else{
        switch( cfg->crc4 ){
        case 0: tmp = 0x00; break;
        case 1: tmp = 0x81; break;
        default:
            printk(KERN_ERR"%s: Error: unexpecter crc4 value = %d\n",MR17G_MODNAME,cfg->crc4);
            goto error;
        }
    }    
    if( pef22554_writereg(chip,chan->num,FMR2,tmp)){
        goto error;
    }

    // XSP: 0x23 - unframed, 0x03 - no CAS, 0x83 - CAS
    if( !cfg->framed ){
        tmp = 0x23;
    }else{
        switch( cfg->cas ){
        case 0: tmp = 0x03; break;
        case 1: tmp = 0x83; break;
        default:
            printk(KERN_ERR"%s: Error: unexpecter cas value = %d\n",MR17G_MODNAME,cfg->crc4);
            goto error;
        }
    }    
    if( pef22554_writereg(chip,chan->num,XSP,tmp)){
        goto error;
    }

    // LIM1: 0x20 - short haul, 0x70 - long haul
    switch( cfg->long_haul ){
    case 0: tmp = 0x20; break;
    case 1: tmp = 0x70; break;
    default:
        printk(KERN_ERR"%s: Error: unexpecter haul value = %d\n",MR17G_MODNAME,cfg->long_haul);
        goto error;
    }
    if( pef22554_writereg(chip,chan->num,LIM1,tmp)){
        goto error;
    }

    // PC5: 0x02 when (!!MXEN || (MXEN && CLKM && CLKR)) else 0x00 
	tmp = ioread8(&regs->MXCR);
    if( !(tmp&MXEN) || ( (tmp&(MXEN|CLKM|CLKR))==(MXEN|CLKM|CLKR)) ){
        tmp = 0x02;
    }else{
        tmp = 0x00;
    }
    if( pef22554_writereg(chip,chan->num,PC5,tmp)){
        goto error;
    }  

    // CMR2: 0x04 when (!!MXEN || (MXEN && CLKM && CLKR)) else 0x00 
	tmp = ioread8(&regs->MXCR);
    if( !(tmp&MXEN) || ( (tmp&(MXEN|CLKM|CLKR))==(MXEN|CLKM|CLKR)) ){
        tmp = 0x04;
    }else{
        tmp = 0x00;
    }
    if( pef22554_writereg(chip,chan->num,CMR2,tmp)){
        goto error;
    }  

	// Loopback configuration
	// Local
	if( pef22554_readreg(chip,chan->num,LIM0,&tmp) ){
		goto error;
	}
	if( cfg->llpb )
		tmp |= 0x02;
	else
		tmp &= (~0x02);
    if( pef22554_writereg(chip,chan->num,LIM0,tmp)){
   	    goto error;
    }  
	// Remote
	if( pef22554_readreg(chip,chan->num,LIM1,&tmp) ){
    	goto error;
	}
	if( cfg->rlpb )
		tmp |= 0x02;
	else
		tmp &= (~0x02);
    if( pef22554_writereg(chip,chan->num,LIM1,tmp)){
   	    goto error;
    }  

    // Reset should be written last
    if( pef22554_writereg(chip,chan->num,CMDR,0x50)){
        goto error;
    }  
	
    
    return 0;
error:
    return -1;
}

int
pef22554_linkstate(struct mr17g_chip *chip, int chnum,u8 framed)
{
    u8 tmp = 0;
    int i, ret = -1;

    for(i=0;i<3;i++){
		u8 link_down = 0;
		if( pef22554_readreg(chip,chnum,FRS0,&tmp) ){
            continue;
        }
		// ???? May be need in condition correction
		if( framed ){
			link_down = (tmp & LOS) || (tmp & LFA);
		}else{
			link_down = (tmp & LOS);
		}
		
        if( link_down ){
            ret = 0;
            goto exit;
        }else{
            ret = 1;
            goto exit;
        }
    }

exit:
    return ret;
}
