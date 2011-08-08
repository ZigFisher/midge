/* sdfe4_lib.c:
 *
 * SDFE4 Library
 *
 *      Infineon SDFE4 SHDSL chipset control library
 *	Provide architecture independent interface for
 *	Sigrand SG-17 devices
 *
 * Authors:
 *      Artem Polyakov <art@sigrand.ru>
 *      Ivan Neskorodev <ivan@sigrand.ru>
 */
	 

#include "include/sg17device.h"
#include "include/sdfe4_lib.h"

//#define DEBUG_ON
#define DEFAULT_LEV 20
#include "sg17debug.h"

inline u32
u8_to_u32(u8 *src)
{
	u32 ret32=0;
	u8 i,*ret8=(u8*)&ret32;
	for( i=0;i<4;i++)
		ret8[i]=src[i];
	return ret32;
}

/*
 * sdfe4_msg_init
 */
int
sdfe4_msg_init(struct sdfe4_msg *msg, char *cmsg, int len)
{
	int i;
	if( len > SDFE4_FIFO8 )
		return -1;
	for( i=0;i<len;i++)
		msg->buf[i] = cmsg[i];
	msg->len = len;
	return 0;
}

/*
 * sdfe4_chk_transplayer:
 * Checks transport layer status provided transport header
 * @msg		- message to check
 */
inline int
sdfe4_chk_transplayer(u8 *msg){
	/* TODO: apply transport layer checking */
	return 0;
}

/*
 * sdfe4_chk_msglayer:
 * Checks message layer  status provided by RSTA register
 * @msg		- message to check
 */
inline int
sdfe4_chk_msglayer(u8 *msg){
	if( msg[0] != 0xA9 ){
		PDEBUG(debug_error,"msg[0]=%02x",msg[0]);
		return -1;
	}
	return 0;
}

/*
 * sdfe4_reset_hwdev_chan:
 * Clear channel status when resetting channel
 * @ch		- channel handler
 */
int
sdfe4_reset_hwdev_chan(struct sdfe4_channel *ch)
{
	int ch_en_bkp=0;
	struct sg17_eoc *e = ch->eoc;	
	PDEBUG(debug_eoc,"EOC before reset: %p",ch->eoc);	
	if( ch->enabled )
		ch_en_bkp=1;
	memset(ch,0,sizeof(*ch));
	ch->enabled=ch_en_bkp;
	ch->eoc = e;
	PDEBUG(debug_eoc,"EOC after reset: %p",ch->eoc);
	return 0;
}

/*
 * sgfe4_xmit_rs_msg:
 * Sends message to RAM block of SDFE-4
 * @opcd	- operation code of RAM command
 * @params	- command parameters
 * @plen	- number of command parameters
 * @ret	- return value (if not NULL)
 * @data	- additional data for portability
 */
int
sdfe4_rs_cmd(u8 opcd, u32 *params, u16 plen,struct sdfe4_ret *ret,struct sdfe4 *hwdev)
{
	int i;
	u32 buf[SDFE4_FIFO32];
	u32 *msg32=(u32*)buf;
	u8  *msg8=(u8*)buf;
	int len=0;
	int r = 0;
	int intr=0;

	if ( plen > FW_PKT_SIZE32+1 ){
		PDEBUG(debug_error,": wrong package size");
		return -1;
	}

	// prepare parameters of message
	// (send without transport protocol)
	msg8[0] = PEF24624_ADR_HOST ;
	msg8[1] = PEF24624_ADR_DEV | PEF24624_ADR_RAMSHELL;
	msg8[2] = 0;
	msg8[3] = opcd;


 	for (i=0; i<plen; i++)
		msg32[i+1]=params[i];

	sdfe4_lock_chip(hwdev);
	//Send message to RAM
	if( (r = sdfe4_hdlc_xmit(msg8,RAM_CMDHDR_SZ+plen*4,hwdev)) ){
		PDEBUG(debug_error,": sdfe4_hdlc_xmit error = %d", r);	
		goto exit;
	}

	
	if( (intr = sdfe4_hdlc_wait_intr(15000,hwdev)) ){
		PDEBUG(debug_error,": sdfe4_hdlc_wait_intr error = %d", r);
//		goto exit;
	}
	if( (r = sdfe4_hdlc_recv((u8*)buf,&len,hwdev)) ){
		PDEBUG(debug_error,": sdfe4_hdlc_recv error = %d", r);	
		goto exit;
	}else if( intr ){
		PDEBUG(debug_error,": sdfe4_hdlc_recv - OK, wait_intr - FAIL");	
	}
	
	msg8=(u8*)&buf;
	msg8+=RAM_ACKHDR_SZ;
	if( sdfe4_chk_msglayer(msg8) ){
		PDEBUG(debug_error,": error in check_msg_layer = %d", r);			
		r = -1;
		goto exit;
	}
	ret->val=u8_to_u32(&msg8[1]);	
	
	if( ret->stamp ){
		
		if( (intr=sdfe4_hdlc_wait_intr(15000,hwdev)) ){
			PDEBUG(debug_error,": sdfe4_hdlc_wait_intr error = %d", r);
//			goto exit;
		}

		if( (r=sdfe4_hdlc_recv((u8*)buf,&len,hwdev)) ){
			PDEBUG(debug_error,": sdfe4_hdlc_recv error = %d", r);			
			goto exit;
		}else if( intr ){
			PDEBUG(debug_error,": sdfe4_hdlc_recv - OK, wait_intr - FAIL");	
		}
	
		msg8=(u8*)&buf;
		msg8+=RAM_ACKHDR_SZ;
		if( (r=sdfe4_chk_msglayer(msg8)) ){
			PDEBUG(debug_error,": error in check_msg_layer = %d", r);			
			goto exit;
		}
		ret->val=u8_to_u32(&msg8[1]);
	}
 exit:
	sdfe4_unlock_chip(hwdev);	
	return r;
}


/*
 * sdfe4_aux_cmd:
 * Sends message to AUX block of SDFE-4
 * @opcd	- operation code of AUX command
 * @params	- command parameter
 * @data	- additional data for portability
 */
int
sdfe4_aux_cmd(u8 opcode, u8 param_1,struct sdfe4_ret *ret,struct sdfe4 *hwdev)
{
	u8 buf[SDFE4_FIFO8];
	u8 *msg8=(u8*)buf;
	int len=0;
	int e = 0;
	int i;
	
	// prepare parameters of message
	// (send without transport protocol)
	msg8[0] = PEF24624_ADR_HOST;
	msg8[1] = PEF24624_ADR_DEV | PEF24624_ADR_AUX;
	msg8[2] = opcode;
	msg8[3] = param_1;
	
	sdfe4_lock_chip(hwdev);

	for(i=0;i<3;i++){ // Try3 times to execute command
		if( (e=sdfe4_hdlc_xmit(msg8,AUX_CMDHDR_SZ+1,hwdev)) ){
			PDEBUG(debug_error,": error in sdfe4_hdlc_xmit");
			continue;
		}

		if( (e = sdfe4_hdlc_wait_intr(15000,hwdev)) ){
			PDEBUG(debug_error,": error in sdfe4_hdlc_wait_intr");
			//continue;
		}
	
		if( (e=sdfe4_hdlc_recv((u8*)buf,&len,hwdev)) ){
			PDEBUG(debug_error,": error in sdfe4_hdlc_recv");
			continue;
		}
		break;
	}
	msg8=(u8*)&buf[RAM_ACKHDR_SZ];
	if( (i == 3) && (e=sdfe4_chk_msglayer(msg8)) ){
		PDEBUG(debug_error,": error in check_msg_layer = %d",e);
		goto exit;
	}
	
	if( ret )
		ret->val=u8_to_u32(&msg8[1]);

 exit:
	sdfe4_unlock_chip(hwdev);
	return e;
}

/*
 * sdfe4_pamdsl_cmd:
 * Sends command to Embedded controller block of SDFE-4
 * @opcd	- operation code of RAM command
 * @ch		- controller number	
 * @params	- command parameters
 * @plen	- number of command parameters
 * @data	- additional data for portability
 */
int
sdfe4_pamdsl_cmd(u8 ch, u16 opcd, u8 *params, u16 plen,struct sdfe4_msg *rmsg,struct sdfe4 *hwdev)
{
	u8 buf[SDFE4_FIFO8];
	u8 *msg8=(u8*)buf;
	int i;
	int error = 0;

	rmsg->len = 0;

	// prepare parameters of message
	msg8[0] = PEF24624_ADR_HOST;
	msg8[1] = PEF24624_ADR_DEV | PEF24624_ADR_PAMDSL(ch);
	// include transport protocol
	msg8[3] = msg8[6] = msg8[7] = 0;
	// add message id
	msg8[4] = opcd & 0xFF;
	msg8[5] = (opcd>>8) & 0xFF;

	// message params must be already in little endian, no conversion !!!
	for(i=0;i<plen;i++)
		msg8[i+8]=params[i];

	sdfe4_lock_chip(hwdev);
	
	for(i=0;i<3;i++){ // Try3 times to execute command
		msg8[2] = 0x08 | ( hwdev->msg_cntr & 0x1);
		if( (error=sdfe4_hdlc_xmit(msg8,EMB_CMDHDR_SZ+plen,hwdev)) ){
			PDEBUG(debug_error,": error in sdfe4_hdlc_xmit. try once more");
			continue;
		}
		
		if( (error=sdfe4_hdlc_wait_intr(15000,hwdev)) ){
			PDEBUG(debug_error,": error opcd(%x) no intr. try once more",opcd);	
			//continue;
		}
	
		if(sdfe4_hdlc_recv(rmsg->buf,&rmsg->len,hwdev) ){
			PDEBUG(debug_error,": error in sdfe4_hdlc_recv. try once more");
			continue;
		}
		break;
	}
		
	if( i==3 || rmsg->ack_id != *(u16*)(&rmsg->buf[4])){
		PDEBUG(debug_error,"pamdsl cmd fails: i=%d",i);
		error = -1;
		goto exit;
	}

 exit:		
	sdfe4_unlock_chip(hwdev);
	return error;
}

/*
 * sdfe4_pamdsl_ack:
 * Sends acknoledge to Embedded controller block of SDFE-4
 * @opcd	- operation code of RAM command
 * @ch	- controller number	
 * @hdr0	- first byte of transport layer header
 * @data	- additional data for portability
 */
int
sdfe4_pamdsl_ack(u8 ch, u8 hdr0,struct sdfe4 *hwdev)
{
	u8 msg[8];

	// prepare parameters of message
	msg[0] = PEF24624_ADR_HOST;
	msg[1] = PEF24624_ADR_DEV | (ch & 0xF);
	// include transport protocol
	msg[2] = hdr0;
	msg[3] = msg[6] = msg[7] = 0;
	// add message id
	msg[4] = 0;
	msg[5] = 0;

	// message params must be already in little endian, no conversion !!!
	if( sdfe4_hdlc_xmit(msg,EMB_ACKHDR_SZ,hwdev) ){
		PDEBUG(debug_error,": error in sdfe4_hdlc_xmit");
		// TODO: error handling
		return -1;
	}
	
	return 0;
}

/*
 * sdfe4_pamdsl_nfc:
 * Proceed notification from Embedded controller block of SDFE-4
 * @msg	- structure, that holds message
 * @hwdev		- structure, that holds information about Embedded channels and entire SDFE4 chip
 */
int
sdfe4_pamdsl_nfc(struct sdfe4_msg *msg,struct sdfe4 *hwdev)
{
	u8 hdr0,hdr1,ch;
	u16 ackID;	

	hdr0=msg->buf[2];
	hdr1=msg->buf[3];
	ch=PEF24624_PAMDSL_ADR(msg->buf[0]);
	PDEBUG(debug_eoc,"ch=%d",ch);
	ackID=*(u16*)(&msg->buf[4]);

	switch( ackID ){
	case NFC_CONNECT_CTRL:
		PDEBUG(debug_sdfe4,"NFC_CONNECT_CTRL, status = %02x",msg->buf[EMB_NFCHDR_SZ]);
		hwdev->ch[ch].state=msg->buf[EMB_NFCHDR_SZ];
		hwdev->ch[ch].state_change=1;
		return 0;
	case NFC_CONNECT_CONDITION:
		hwdev->ch[ch].conn_state=msg->buf[EMB_NFCHDR_SZ];
		hwdev->ch[ch].conn_state_change=1;
		return 0;
	case NFC_SDI_DPLL_SYNC:
		hwdev->ch[ch].sdi_dpll_sync=1;
		PDEBUG(debug_sdfe4,"sdi_dpll_sync=1");
		return 0;
	case NFC_PERF_PRIM:
		hwdev->ch[ch].perf_prims=msg->buf[EMB_NFCHDR_SZ];
		return 0;
	case NFC_EOC_TX:
		PDEBUG(debug_eoc,"EOC(%d): new state: (%p) msg->len=%d",ch,hwdev->ch[ch].eoc,msg->len);
		hwdev->ch[ch].eoc->eoc_tx=msg->buf[EMB_NFCHDR_SZ];
		return 0;
	case NFC_EOC_RX:
		PDEBUG(debug_eoc,"EOC(%d): Get new message",ch);
		sdfe4_eoc_init(&hwdev->ch[ch],&msg->buf[EMB_NFCHDR_SZ],msg->len-EMB_NFCHDR_SZ);
		return 0;
	case NFC_UNDEF_MSG_ID:
	case NFC_MULTIWIRE_MASTER:
	case NFC_MULTIWIRE_PAIR_NR:
	case NFC_MPAIR_DELAY_MEASURE_SDFE4:	
	case NFC_FBIT_RX:
		break;
	}
	return -1;
}

/*
 * sdfe4_pamdsl_parse:
 * Parse message from SDFE-4 chipset
 * @rmsg 	- structure, that holds message
 * @hwdev	- structure, that holds information about Embedded channels and entire SDFE4 chip
 * return message type
 */
int
sdfe4_pamdsl_parse(struct sdfe4_msg *rmsg,struct sdfe4 *hwdev)
{
	u8 hdr0,hdr1,chan;

	hdr0=rmsg->buf[2];
	hdr1=rmsg->buf[3];
	chan=rmsg->buf[0];
	
	if( (chan & 0x1) ){
		// PDEBUG(debug_sdfe4,"Error channel - %02x",chan);
		return  SDFE4_NOT_PAMDSL;
	}

	switch ( hdr0 & 0xfe ){
	case 0x88:
		// Transport counter sync
		if( ( (hwdev->msg_cntr) & 0x1) != (hdr0 & 0x1) ){
			hwdev->msg_cntr = (hdr0 & 0x1);
		}
		(hwdev->msg_cntr)++;
		return SDFE4_PAMDSL_SYNC;
	case 0x06:
		// Reset requested from the counter part.
		(hwdev->msg_cntr)=0;		
		return SDFE4_PAMDSL_SYNC;
	case 0x08:
		// This CMD, ACK or NFC
		sdfe4_pamdsl_ack(chan, (0x88 | ( hdr0 & 0x1)),hwdev);
		if( hdr1 ){
		 	hwdev->msg_cntr++;	
			sdfe4_pamdsl_nfc(rmsg,hwdev);
			return SDFE4_PAMDSL_NFC;
		}
		return SDFE4_PAMDSL_ACK;
	case 0x8E:
		//   Counter part received corrupted message.
		hwdev->msg_cntr++;
		return SDFE4_PAMDSL_SYNC;
	case 0x00:
	default:
		// TODO:  Error handling
		return SDFE4_PAMDSL_ERROR;
	}
}


/*
 * sdfe4_drv_poll:
 * Poll and proceed the messages from Embedded controller block of SDFE-4
 * @rmsg		- structure, that will hold received message
 * @hwdev		- structure, that holds information about Embedded channels and entire SDFE4 chip
 */
int
sdfe4_drv_poll(struct sdfe4_msg *rmsg,struct sdfe4 *hwdev)
{
	//,ackID
	while(1){
		if( sdfe4_hdlc_wait_intr(150000,hwdev) )
			return -1;
		if( sdfe4_hdlc_recv(rmsg->buf,&rmsg->len,hwdev) ){
			// TODO: error handling
			return -1;
		}
		if( sdfe4_pamdsl_parse(rmsg,hwdev) > 0 )
			break;
	}
	return 0;	
}


/*
 * sdfe4_download_fw:
 * Downloads firmware to SDFE-4 chipset
 * @hwdew	- structure, that holds information about Embedded channels and entire SDFE4 chip
 * @fw		- (only for PCI adapter) - pointer to firmware 
 * @fw_size	- (only for PCI adapter) - firmware size
 */
int
sdfe4_download_fw(struct sdfe4 *hwdev
#ifdef SG17_PCI_MODULE
				  , u8 *fw, int fw_size
#endif
				  )
{
	int i,k,iter;
  	u32 Data_U32[256];
  	struct sdfe4_ret ret;
#ifdef SG17_REPEATER	
	struct sdfe4_msg rmsg;
#endif	
	u8 *ret8;
	// chipset firmware CRC-s
	u8 CODE_CRC[4];	
	u8 DATA_CRC[4];
	u32 code_size, data_size, data_offs, code_offs;
	u32 fw_dtpnt;

	switch(hwdev->type){
	case SDFE4v1:
        for(i=0;i<4;i++){
            CODE_CRC[i] = fw[FW_CODE_CRC_OFFS+3-i];
		    DATA_CRC[i] = fw[FW_DATA_CRC_OFFS+3-i];
        }
		code_size = FW_CODE_SIZE;
		data_size = FW_DATA_SIZE;
		code_offs = FW_CODE_OFFS;
		data_offs = FW_DATA_OFFS;
		fw_dtpnt = FWdtpnt;		
		break;
	case SDFE4v2:
        for(i=0;i<4;i++){
            CODE_CRC[i] = fw[FW_CODE_CRC_OFFSv2+3-i];
		    DATA_CRC[i] = fw[FW_DATA_CRC_OFFSv2+3-i];
        }
		code_size = FW_CODE_SIZEv2;
		data_size = FW_DATA_SIZEv2;
		code_offs = FW_CODE_OFFSv2;
		data_offs = FW_DATA_OFFSv2;
		fw_dtpnt = FWdtpnt_v2;		
		break;
	default:
		PDEBUG(debug_error,"Unknown device type: %d",hwdev->type);
		return -1;
	}
	wait_ms(2);

// TODO: remove
//    printk(KERN_NOTICE"Result CRC values. CODE: ");
//    for(i=0;i<4;i++){
//           printk("%02x:",CODE_CRC[i]);
//    }
//    printk(" DATA: ");
//    for(i=0;i<4;i++){
///           printk("%02x:",DATA_CRC[i]);
//    }
//    printk("\n");
// --- REMOVE

	PDEBUG(debug_sdfe4,"hwdev = %08x,type=%d",(u32)hwdev,hwdev->type);
	PDEBUG(debug_sdfe4,"hwdev->data = %08x",(u32)hwdev->data);	
	Data_U32[0]=0;
	ret.stamp=0;
	i = 0;
	while( sdfe4_rs_cmd(CMD_WR_REG_RS_FWSTART,Data_U32,1,&ret,hwdev) && (i<3) ){
		PDEBUG(debug_error,": error in CMD_WR_REG_RS_FWSTART(FIRST), try #%d",i);
		wait_us(100);
		i++;
	}

	if( i == 3 ){
		PDEBUG(debug_error,": FATAL error in CMD_WR_REG_RS_FWSTART(FIRST) - ABORT");
		return -1;
	}

	if( sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL,Data_U32,1,&ret,hwdev)){
		PDEBUG(debug_error,": error in CMD_WR_REG_RS_FWCTRL(SECOND)");
		return -1;
	}

 	iter = code_size/FW_PKT_SIZE;
	for(k=0;k<iter;k++){
		Data_U32[0]=(k*FW_PKT_SIZE)/4;
		for(i=0;i<FW_PKT_SIZE/4;i++){
#ifdef SG17_PCI_MODULE
			Data_U32[i+1] = cpu_to_be32(*((u32*)&fw[k*FW_PKT_SIZE + i*4]));
#else
			Data_U32[i+1]=cpu_to_be(FLASH_WordRead (FLASH_FW_STRTADDR + code_offs +
													FW_PKT_SIZE*k + i*0x4));
#endif								
		}
		if( sdfe4_rs_cmd(CMD_WR_RAM_RS,(u32*)Data_U32,FW_PKT_SIZE32+1,&ret,hwdev)){
			PDEBUG(debug_error,": error in CMD_WR_RAM_RS for CODE, iter = %d",k);
			return -1;
		}
		wait_us(30);
	}

	wait_ms(2);

	ret.stamp = 1;
	ret8 = (u8*)&ret.val;
	Data_U32[0]=FWCTRL_CHK;
	if( sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL,Data_U32,1,&ret,hwdev)){
		PDEBUG(debug_error,": error in CMD_WR_REG_RS_FWCTRL(FWCTRL_CHK) for CODE");	
		wait_ms(2);
		Data_U32[0]=0;
		ret.stamp=0;
		if( sdfe4_rs_cmd(CMD_RD_REG_RS_FWCRC,Data_U32,1,&ret,hwdev)){
			PDEBUG(debug_error,": error in CMD_RD_REG_RS_FWCRC for CODE");			
			return -1;
		}
	}


	//  Count firmware code CRC 
	PDEBUGL(debug_sdfe4,"Code CRC: ");
	for(i=0;i<4;i++){
		PDEBUGL(debug_sdfe4,"%02x ",ret8[i]);
	}
	PDEBUGL(debug_sdfe4,"\n");
	
	for(i=0;i<4;i++){
		PDEBUG(debug_sdfe4,"CODE CRC, check %d byte: USE=%02x, IS=%02x",i,CODE_CRC[i],ret8[i]);
		if(CODE_CRC[i]!=ret8[i]){
			PDEBUG(debug_sdfe4,"CODE CRC mismatch");
			return -1;
		}
	}


PDEBUG(debug_sdfe4,"Load firmware");


	// Load firmware data 
	wait_ms(100);
	ret.stamp=0;
	Data_U32[0]=FWCTRL_SWITCH;
	if( sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL,Data_U32,1,&ret,hwdev) ){
		PDEBUG(debug_error,": error in CMD_WR_REG_RS_FWCTRL(FWCTRL_SWITCH) for DATA");
		return -1;
	}

	wait_ms(100);
	iter = data_size/FW_PKT_SIZE;
  	for(k=0;k<iter;k++){
		Data_U32[0]=(k*FW_PKT_SIZE)/4;
		for(i=0;i<FW_PKT_SIZE/4;i++){
#ifdef SG17_PCI_MODULE
			Data_U32[i+1] = cpu_to_be32(*((u32*)&fw[data_offs + k*FW_PKT_SIZE + i*4]));
#else
			Data_U32[i+1] = cpu_to_be(FLASH_WordRead (FLASH_FW_STRTADDR + data_offs +
                                			          FW_PKT_SIZE*k + i*0x4));
#endif								  
		}
		if( sdfe4_rs_cmd(CMD_WR_RAM_RS,(u32*)Data_U32,FW_PKT_SIZE32+1,&ret,hwdev)){
			PDEBUG(debug_error,": error in CMD_WR_RAM_RS for DATA, iter=%d",k);
			return -1;
		}
		wait_us(30);
	}

 	wait_ms(2);
	ret.stamp=1;
	Data_U32[0]=FWCTRL_CHK | FWCTRL_SWITCH;
	if( sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL,Data_U32,1,&ret,hwdev)){
		PDEBUG(debug_error,": error in CMD_WR_REG_RS_FWCTRL(FWCTRL_CHK|FWCTRL_SWITCH) for DATA");
		wait_ms(2);
		Data_U32[0]=0;
		ret.stamp=0;
		if( sdfe4_rs_cmd(CMD_RD_REG_RS_FWCRC,Data_U32,1,&ret,hwdev)){
			PDEBUG(debug_error,": error in CMD_RD_REG_RS_FWCRC for DATA");	
			return -1;
		}
	}

	// Count firmware data CRC DATA
	PDEBUGL(debug_sdfe4,"Data CRC: ");
	for(i=0;i<4;i++){
		PDEBUGL(debug_sdfe4,"%02x ",ret8[i]);
	}
	PDEBUGL(debug_sdfe4,"\n");
	

	for(i=0;i<4;i++){
		if(DATA_CRC[i]!=ret8[i]){
			PDEBUG(debug_error,"DATA CRC mismatch");
			return -1;
		}
	}

	wait_ms(100);
	ret.stamp=0;
	Data_U32[0]=fw_dtpnt;
  	if( sdfe4_rs_cmd(CMD_WR_REG_RS_FWDTPNT,Data_U32,1,&ret,hwdev)){
		PDEBUG(debug_error,": error in CMD_WR_REG_RS_FWDTPNT");	
		return -1;
	}

	wait_ms(200);
	Data_U32[0]=FWCTRL_VALID;
  	if( sdfe4_rs_cmd(CMD_WR_REG_RS_FWCTRL,Data_U32,1,&ret,hwdev)){
		PDEBUG(debug_error,": error in CMD_WR_REG_RS_FWCTRL(FWCTRL_VALID)");		
		return -1;
	}

  	wait_ms(200);
	Data_U32[0]=0;
	for(i=0;i<SDFE4_EMB_NUM;i++){
		if( hwdev->ch[i].enabled )
			Data_U32[0] |= (1<<i);
	}
	
	i = 0;
  	while(sdfe4_rs_cmd(CMD_WR_REG_RS_FWSTART,Data_U32,1,&ret,hwdev) && (i<3)){
		PDEBUG(debug_error,": error in CMD_WR_REG_RS_FWSTART(LAST), try#%d",i);
		wait_ms(2);
		i++;
	}

	if( i == 3 ){
		PDEBUG(debug_error,": FATAL error in CMD_WR_REG_RS_FWSTART(LAST)");
		return -1;
	}


	// INIT
#ifdef SG17_REPEATER	
	while( !sdfe4_drv_poll(&rmsg,hwdev) );
#endif	
	
	for(i=0;i<SDFE4_EMB_NUM;i++){
		if( hwdev->ch[i].enabled &&
			hwdev->ch[i].state != MAIN_INIT ){
			return -1;
		}
		hwdev->ch[i].state_change=0;
	}
	return 0;
}

/*
 * sdfe4_setup_chan:
 * Setup SHDSL Embedded controller block of SDFE-4
 * @ch		- Embedded controller number
 * @hwdev	- structure, that holds information about Embedded channels and entire SDFE4 chip
 * return error status
 */
int
sdfe4_setup_chan(u8 ch, struct sdfe4 *hwdev)
{
	struct sdfe4_if_cfg *cfg=&(hwdev->cfg[ch]);
   	u32 buf[SDFE4_FIFO32];
	struct cmd_cfg_sym_dsl_mode *sym_dsl;
	struct cmd_cfg_ghs_mode *ghs_mode;
	struct cmd_cfg_caplist_short_ver_2 *caplist;
	struct cmd_cfg_sdi_settings *sdi_settings;
	struct cmd_cfg_sdi_tx *sdi_tx;
	struct cmd_cfg_sdi_rx *sdi_rx;
	struct cmd_cfg_eoc_rx *eoc_rx;
	struct ghs_ns_field *ns_field_set;
	struct cmd_cfg_ghs_extended_pam_mode *extended_pam_mode;
	struct sdfe4_msg rmsg;
	struct sdfe4_ret ret;
	int i;

	// 0. Prepare
	if( cfg->mode == STU_R ){
	    cfg->annex = ANNEX_A_B;
	}

	// 1. Setup if role
	PDEBUG(debug_sdfe4,"Setup if role");
	sym_dsl=(struct cmd_cfg_sym_dsl_mode *)buf;
	memset(sym_dsl,0,sizeof(*sym_dsl));
	sym_dsl->repeater= cfg->repeater ;
	sym_dsl->mode=cfg->mode;
	sym_dsl->standard=SHDSL;
	rmsg.ack_id=ACK_CFG_SYM_DSL_MODE;
	if(sdfe4_pamdsl_cmd(ch,CMD_CFG_SYM_DSL_MODE,(u8*)buf,sizeof(*sym_dsl),&rmsg,hwdev)){
		PDEBUG(debug_error,": error in CMD_CFG_SYM_DSL_MODE");
		return -1;
	}
	
	
	//2. Setup transaction
	PDEBUG(debug_sdfe4,"Setup transaction");
	ghs_mode=(struct cmd_cfg_ghs_mode *)buf;
	memset(ghs_mode,0,sizeof(*ghs_mode));
	ghs_mode->transaction = cfg->transaction;
	ghs_mode->startup_initialization=cfg->startup_initialization;
	ghs_mode->pmms_margin_mode=PMMS_NORMAL;
	// PBO influence
	if( cfg->mode == STU_C ){
		switch( cfg->pbo_mode ){
		case PWRBO_NORMAL:
			ghs_mode->pbo_mode=PBO_NORMAL;
			ghs_mode->epl_mode=EPL_ENABLED;
			break;
		case PWRBO_FORCED:
			ghs_mode->pbo_mode=PBO_FORCED;
			ghs_mode->epl_mode=EPL_DISABLED;
			break;
		}
	}else{
		ghs_mode->pbo_mode=PBO_NORMAL;
		ghs_mode->epl_mode=EPL_ENABLED;
	}		
	rmsg.ack_id=ACK_CFG_GHS_MODE;

	if(sdfe4_pamdsl_cmd(ch,CMD_CFG_GHS_MODE,(u8*)buf,sizeof(*ghs_mode),&rmsg,hwdev)){
		PDEBUG(debug_error,": error in CMD_CFG_GHS_MODE");
		return -1;
	}
		
	// 3 Caplist_V2
	PDEBUG(debug_sdfe4,"Caplist_V2");
	caplist=(struct cmd_cfg_caplist_short_ver_2 *)buf;
	memset(caplist,0,sizeof(*caplist));

	caplist->pow_backoff=0x00;
	caplist->clock_mode=SHDSL_CLK_MODE_1|SHDSL_CLK_MODE_2|SHDSL_CLK_MODE_3a;

	if( cfg->mode == STU_C ){
		switch( cfg->clkmode ){
		case 0:
			caplist->clock_mode=SHDSL_CLK_MODE_1;
			break;
		case 1:
			//	printk(KERN_NOTICE"Configure sync mode\n");
			caplist->clock_mode=SHDSL_CLK_MODE_3a;
			break;
		case 2:
			caplist->clock_mode=SHDSL_CLK_MODE_2;
			break;
		}


		// PBO influence	
		if( cfg->pbo_mode == PWRBO_FORCED ){
			if( cfg->pbo_vnum )
				caplist->pow_backoff = cfg->pbo_vals[0];
			else
				caplist->pow_backoff = 0;
		}
	}

	caplist->annex = cfg->annex;
	caplist->psd_mask=0x00;

	switch( hwdev->type ){
	case SDFE4v1:
		if(cfg->mode==STU_R){
			caplist->base_rate_min=192;
			caplist->base_rate_max=2304;
			caplist->base_rate_min16=2368;
			caplist->base_rate_max16=3840;
			caplist->base_rate_min32=768;
			caplist->base_rate_max32=5696;
		}else {
			if( cfg->tc_pam == TCPAM32 ){
				caplist->base_rate_min=0;
				caplist->base_rate_max=0;
				caplist->base_rate_min16=0;
				caplist->base_rate_max16=0;
				caplist->base_rate_min32=TCPAM32_INT1_MIN;
				caplist->base_rate_max32=cfg->rate;
			}else if( cfg->tc_pam == TCPAM16 ){
				caplist->base_rate_min32=0;
				caplist->base_rate_max32=0;
				if( cfg->rate > TCPAM16_INT1_MAX ){
					caplist->base_rate_min=TCPAM16_INT1_MIN;
					caplist->base_rate_max=TCPAM16_INT1_MAX;
					caplist->base_rate_min16=TCPAM16_INT2_MIN;
					caplist->base_rate_max16=cfg->rate;
				}else{
					caplist->base_rate_min16=0;
					caplist->base_rate_max16=0;
					caplist->base_rate_min=TCPAM16_INT1_MIN;
					caplist->base_rate_max=cfg->rate;
				}			
			}	
		}
		break;
	case SDFE4v2:
		caplist->base_rate_min=192;
		caplist->base_rate_max=2304;
		caplist->base_rate_min16=2368;
		caplist->base_rate_max16=3840;
	 	caplist->base_rate_min32=768;
		caplist->base_rate_max32=5696;
	default:
		PDEBUG(debug_error,"Unknown device type");
	}
	caplist->sub_rate_min=0x00;
	caplist->sub_rate_max=0x00;
	caplist->enable_pmms=PMMS_OFF;
	caplist->pmms_margin=0x00;

	rmsg.ack_id = ACK_CFG_CAPLIST_SHORT_VER_2;
	if(sdfe4_pamdsl_cmd(ch,CMD_CFG_CAPLIST_SHORT_VER_2,(u8*)buf,sizeof(*caplist),&rmsg,hwdev)){
		PDEBUG(debug_error,": error in CMD_CFG_CAPLIST_SHORT_VER_2");	
		return -1;
	}		

	// 4 
	if( cfg->mode == STU_C ){
		ns_field_set=(struct ghs_ns_field*)buf;
		memset(ns_field_set,0,sizeof(*ns_field_set));
		switch( cfg->pbo_mode ){
		case PWRBO_NORMAL:
			ns_field_set->valid_ns_data = SDI_NO;
			break;
		case PWRBO_FORCED:
			if( cfg->pbo_vnum > 1 ){
				ns_field_set->valid_ns_data = SDI_YES;
				for(i=1;i<cfg->pbo_vnum;i++){
					ns_field_set->ns_info[i-1] = cfg->pbo_vals[i];
				}
				ns_field_set->ns_info_len = cfg->pbo_vnum-1;
			}else{
				ns_field_set->valid_ns_data = SDI_NO;
			}				
			break;
		}
		rmsg.ack_id = ACK_CFG_GHS_NS_FIELD;
		if(sdfe4_pamdsl_cmd(ch,CMD_CFG_GHS_NS_FIELD,(u8*)buf,sizeof(*ns_field_set),&rmsg,hwdev)){
			PDEBUG(debug_error,": error in CMD_CFG_GHS_NS_FIELD");	
			return -1;
		}		
	}

	// 5 config AUX
	if (ch == 0){ 
		PDEBUG(debug_sdfe4,"config AUX_SDI_IF_SEL_0");		
		wait_ms(20);
		if(sdfe4_aux_cmd(CMD_WR_REG_AUX_SDI_IF_SEL_0,0x00+0x04, &ret,hwdev)){  // +0x04 - REFCLK input
			PDEBUG(debug_error,": error in CMD_WR_REG_AUX_SDI_IF_SEL_0");		
			return -1;
		}
	}
	if (ch == 3){
		PDEBUG(debug_sdfe4,"config AUX_SDI_IF_SEL_3");
		wait_ms(20);
		if(sdfe4_aux_cmd(CMD_WR_REG_AUX_SDI_IF_SEL_3,0x03+0x04, &ret,hwdev)){  // +0x04 - REFCLK input
			PDEBUG(debug_error,": error in CMD_WR_REG_AUX_SDI_IF_SEL_3");		
			return -1;
		}
	}	
		
	PDEBUG(debug_sdfe4,"config AUX_AUX_IF_MODE");
	wait_ms(20);
	if(sdfe4_aux_cmd(CMD_WR_REG_AUX_AUX_IF_MODE,0x82, &ret,hwdev)){
		PDEBUG(debug_error,": error in CMD_WR_REG_AUX_AUX_IF_MODE");		
		return -1;
	}
	
	
	//6 SDI settings
	PDEBUG(debug_sdfe4,"SDI settings");
	wait_ms(20);
	sdi_settings=(struct cmd_cfg_sdi_settings*)buf;
	memset(sdi_settings,0,sizeof(*sdi_settings));
	sdi_settings->input_mode=SDI_TDMCLK_TDMSP ;
	sdi_settings->output_mode=SDI_TDMSP_TDMMSP;
	sdi_settings->frequency=cfg->frequency;
	sdi_settings->payload_bits=cfg->payload_bits;
	sdi_settings->frames=0x30;
	sdi_settings->loop=cfg->loop;
	sdi_settings->ext_clk8k=SDI_NO;
	switch( hwdev->type ){
	case SDFE4v1:
		sdi_settings->dpll4bclk=SDI_NODPLL;
		break;
	case SDFE4v2:
		sdi_settings->dpll4bclk=SDI_DPLL4IN;
		break;
	default:
		PDEBUG(debug_error,"Unknown device type");
	}
	sdi_settings->refclkin_freq=TIM_REF_CLK_IN_8192KHZ;
	sdi_settings->refclkout_freq=TIM_DATA_CLK_8192KHZ;
	rmsg.ack_id=ACK_CFG_SDI_SETTINGS;
	if(sdfe4_pamdsl_cmd(ch,CMD_CFG_SDI_SETTINGS,(u8*)buf,sizeof(*sdi_settings),&rmsg,hwdev)){
		PDEBUG(debug_error,": error in CMD_CFG_SDI_SETTINGS");		
		return -1;
	}		

	//7 Config SDI RX
	PDEBUG(debug_sdfe4,"Config SDI RX");
	sdi_rx=(struct cmd_cfg_sdi_rx*)buf;
	memset(sdi_rx,0,sizeof(*sdi_rx));
	sdi_rx->frame_shift=0x00;
	sdi_rx->sp_level=SDI_HIGH;
	sdi_rx->driving_edg=SDI_RISING;
	sdi_rx->data_shift_edg=SDI_NO;
	switch( hwdev->type ){
	case SDFE4v1:
		sdi_rx->lstwr_1strd_dly=0x93;
		break;
	case SDFE4v2:
		sdi_rx->lstwr_1strd_dly=0x64;
		break;
	default:
		PDEBUG(debug_error,"Unknown device type");
	}
	sdi_rx->slip_mode=SLIP_FAST;
	sdi_rx->align=SDI_NO;
	rmsg.ack_id=ACK_CFG_SDI_RX;
	if(sdfe4_pamdsl_cmd(ch,CMD_CFG_SDI_RX,(u8*)buf,sizeof(*sdi_rx),&rmsg,hwdev)){
		PDEBUG(debug_error,": error in CMD_CFG_SDI_RX");	
		return -1;
	}
		
	//8 Config SDI TX
	PDEBUG(debug_sdfe4,"Config SDI TX");
	sdi_tx=(struct cmd_cfg_sdi_tx*)buf;
	memset(sdi_tx,0,sizeof(*sdi_tx));
	sdi_tx->frame_shift=0x00;
	sdi_tx->sp_level=SDI_HIGH;
	sdi_tx->sp_sample_edg=SDI_FALLING;
	sdi_tx->data_sample_edg=SDI_FALLING;
	switch( hwdev->type ){
	case SDFE4v1:
		sdi_tx->lstwr_1strd_dly=0x93;
		break;
	case SDFE4v2:
		sdi_tx->lstwr_1strd_dly=0x16;
		break;
	default:
		PDEBUG(debug_error,"Unknown device type");
	}
	sdi_tx->slip_mode=SLIP_FAST;
	sdi_tx->align=SDI_NO;
	rmsg.ack_id=ACK_CFG_SDI_TX;
	if(sdfe4_pamdsl_cmd(ch,CMD_CFG_SDI_TX,(u8*)buf,sizeof(*sdi_tx),&rmsg,hwdev)){
		PDEBUG(debug_error,": error in CMD_CFG_SDI_TX");	
		return -1;
	}

	//9. config EOC
	PDEBUG(debug_sdfe4,"Config EOC");
	eoc_rx=(struct cmd_cfg_eoc_rx*)buf;
	memset(eoc_rx,0,sizeof(*eoc_rx));
	eoc_rx->max_num_bytes=800;
	rmsg.ack_id=ACK_CFG_EOC_RX;
	if(sdfe4_pamdsl_cmd(ch,CMD_CFG_EOC_RX,(u8*)buf,sizeof(*eoc_rx),&rmsg,hwdev)){
		PDEBUG(debug_error,": error in CMD_CFG_EOC_RX");	
		return -1;
	}
	
    //10. Extended pam mode (only for v2 chipsets)
	if( hwdev->type == SDFE4v2 ){
		extended_pam_mode=(struct cmd_cfg_ghs_extended_pam_mode*)buf;
   		memset(extended_pam_mode,0,sizeof(*extended_pam_mode));
		extended_pam_mode->ext_pam_mode=0x1; 
		if(cfg->mode==STU_R){
			extended_pam_mode->bits_per_symbol=TCPAM128;
			extended_pam_mode->speed_rate=12680;	
		}else {
			extended_pam_mode->bits_per_symbol=cfg->tc_pam;
			if(cfg->tc_pam==TCPAM128){
				extended_pam_mode->speed_rate=cfg->rate+8;
			}else {
				extended_pam_mode->speed_rate=cfg->rate;
			}
		}
	   	rmsg.ack_id=ACK_CFG_GHS_EXTENDED_PAM_MODE;
		if(sdfe4_pamdsl_cmd(ch,CMD_CFG_GHS_EXTENDED_PAM_MODE,(u8*)buf,sizeof(*extended_pam_mode),&rmsg,hwdev)){
			//PDEBUG(debug_error,": error in CMD_CFG_EOC_RX");	
			return -1;
		}
	}
	return 0;
}

/*
 * sdfe4_setup_channel:
 * Wrapper to sdfe4_setup_chan
 * @ch		- Embedded controller number
 * @hwdev	- structure, that holds information about Embedded channels and entire SDFE4 chip
 * return error status
 */
inline int
sdfe4_setup_channel(int ch, struct sdfe4 *hwdev)
{
	sdfe4_clear_channel(hwdev);
	return sdfe4_setup_chan(ch,hwdev);
}

/*
 * sdfe4_start_channel:
 * Starup Embedded controller block of SDFE-4
 * @ch		- Embedded controller number
 * @hwdev	- structure, that holds information about Embedded channels and entire SDFE4 chip
 * return error status
 */
inline int
sdfe4_start_channel(int ch,struct sdfe4 *hwdev)
{
	struct sdfe4_msg rmsg;
	u8 main_pre_act[3]={MAIN_PRE_ACT,0x0,0x0};
	wait_ms(10);
	rmsg.ack_id=ACK_CONNECT_CTRL;
	if(sdfe4_pamdsl_cmd(ch,CMD_CONNECT_CTRL,main_pre_act,3,&rmsg,hwdev))
		return -1;
		
	if( rmsg.buf[EMB_ACKHDR_SZ] != MAIN_PRE_ACT )
		return -1;
	return 0;
}


/*
 * sdfe4_disable_channel:
 * Disables Embedded controller block of SDFE-4
 * @ch		- Embedded controller number
 * @hwdev	- structure, that holds information about Embedded channels and entire SDFE4 chip
 * return error status
 */
inline int
sdfe4_disable_channel(int ch,struct sdfe4 *hwdev)
{
	struct sdfe4_msg rmsg;
	u8 main_init[3]={MAIN_INIT,0x0,0x0};	
	wait_ms(50);
	rmsg.ack_id=ACK_CONNECT_CTRL;
	if(sdfe4_pamdsl_cmd(ch,CMD_CONNECT_CTRL,main_init,3,&rmsg,hwdev))
		return -1;

	return 0;
}

/*
 * sdfe4_start_as_modem:
 * Starts enabled Embedded controller blocks of SDFE-4 in modem mode
 * @hwdev		- structure, that holds information about Embedded channels and entire SDFE4 chip
 * return error status
 */
inline int
sdfe4_start_as_modem(struct sdfe4 *hwdev)
{
	int i;
	for(i=0;i<SDFE4_EMB_NUM;i++){
		if( hwdev->ch[i].enabled ){
			if( sdfe4_start_channel(i,hwdev) )
				return -1;
#ifdef SG17_PCI_MODULE
			wait_ms(10);
#endif
		}
	}
	return 0;
}

/*
 * sdfe4_load_config:
 * Sync parameters of Embedded controller block and device handler (hwdev)
 * @ch		- Embedded controller number
 * @hwdev	- structure, that holds information about Embedded channels and entire SDFE4 chip
 * return error status
 */
int
sdfe4_load_config(u8 ch, struct sdfe4 *hwdev)
{
  	struct sdfe4_msg rmsg;
	struct ack_dsl_param_get *dsl_par;
	struct sdfe4_if_cfg *ch_cfg=&(hwdev->cfg[ch]);
	struct cmd_ghs_cap_get *cmd_cap_get;
	struct ack_ghs_cap_get *ack_cap_get;
	int TC_PAM = TCPAM16;
	int r;
	u8 buf[sizeof(*cmd_cap_get)];

	wait_ms(10);
	rmsg.ack_id=ACK_DSL_PARAM_GET;
	if( (r = sdfe4_pamdsl_cmd(ch,CMD_DSL_PARAM_GET,NULL,0,&rmsg,hwdev)) ){
		PDEBUG(debug_error,"error(%d) in CMD_DSL_PARAM_GET",r);
		return -1;
	}
	dsl_par=(struct ack_dsl_param_get *)&(rmsg.buf[8]);
	PDEBUG(debug_sdfe4,"Get return");	
	
	// Chipset version differentiation
	switch( hwdev->type ){
	case SDFE4v1:
		if(dsl_par->bits_p_symbol >= 0x04){
			TC_PAM =TCPAM32;
		}else{
			TC_PAM =TCPAM16;
		}
		break;
	case SDFE4v2:
		TC_PAM = dsl_par->bits_p_symbol;
		break;
	default:
		PDEBUG(debug_error,"Unknown device type");
	}
	
	ch_cfg->annex = dsl_par->annex;
	ch_cfg->tc_pam = TC_PAM;
	ch_cfg->rate = dsl_par->base_rate;

	cmd_cap_get = (struct cmd_ghs_cap_get *)buf;
	memset(cmd_cap_get,0,sizeof(* cmd_cap_get));
	cmd_cap_get->ClType=REMOTE;	

	if(ch_cfg->annex == ANNEX_A){
		cmd_cap_get->ClParam=TPS_TC_A;
	}else{
		cmd_cap_get->ClParam=TPS_TC_B;
	}
	
	rmsg.ack_id=ACK_GHS_CAP_GET;
	if( (r = sdfe4_pamdsl_cmd(ch,CMD_GHS_CAP_GET,(u8*)buf,sizeof(*cmd_cap_get),&rmsg,hwdev)) ){
		PDEBUG(debug_error,"error(%d) in CMD_GHS_CAP_GET",r);
		return -1;
	}

	ack_cap_get=(struct ack_ghs_cap_get *)&(rmsg.buf[8]);

	switch(ack_cap_get->ClData[0]){
	case SHDSL_CLK_MODE_1:
		ch_cfg->clkmode = 0;  // plesiochronous
		break;
	case SHDSL_CLK_MODE_2:
		ch_cfg->clkmode = 2;  // plesiochronous with timing reference
		break;
	case SHDSL_CLK_MODE_3a:
		ch_cfg->clkmode = 1;  // synchronous
		break;
	}
	
	PDEBUG(debug_sdfe4,"rate = %d",ch_cfg->rate);	
	return 0;
}

int
sdfe4_get_statistic(u8 ch, struct sdfe4 *hwdev,struct sdfe4_stat *stat)
{
  	struct sdfe4_msg rmsg;
	struct ack_perf_status_get *perf;
	//	struct ack_dsl_param_get *dsl_par;		
	//	struct sdfe4_if_cfg *ch_cfg=&(hwdev->cfg[ch]);
	int r;
	
	wait_ms(10);

	rmsg.ack_id=ACK_PERF_STATUS_GET;
	if( (r = sdfe4_pamdsl_cmd(ch,CMD_PERF_STATUS_GET,NULL,0,&rmsg,hwdev)) ){
		PDEBUG(debug_error,"error(%d) in CMD_DSL_PARAM_GET",r);
		return -1;
	}
	perf = (struct ack_perf_status_get*)&(rmsg.buf[8]);
	sdfe4_memcpy(stat,perf,sizeof(*perf));
	return 0;
}


int
sdfe4_flush_state(struct sdfe4 *hwdev)
{
	struct sdfe4_channel *chan;
	int i;
	
	PDEBUG(debug_sdfe4,"");
	for(i=0;i<4;i++){
		if( !hwdev->ch[i].enabled )
			continue;
		chan=&hwdev->ch[i];
		chan->state_change = 0;
		chan->state = 0;
		chan->conn_state = 0;
		chan->state_change = 0;
	}
	return 0;
}

/*
 * sdfe4_state_mon:
 * Process notifications from Embedded controller block of SDFE4
 * @hwdev	- structure, that holds information about Embedded channels and entire SDFE4 chip
 * return error status
 */
int
sdfe4_state_mon(struct sdfe4 *hwdev)
{
	int i;
	struct sdfe4_channel *chan;
	struct sdfe4_if_cfg *cfg;
	
	PDEBUG(debug_sdfe4,"");
	for(i=0;i<4;i++){
		if( !hwdev->ch[i].enabled )
			continue;
		chan=&hwdev->ch[i];
		cfg=&hwdev->cfg[i];
		if( chan->state_change ){
			chan->state_change=0; // Make ATOMIC ??
			switch( chan->state ){
			case MAIN_CORE_ACT:
				sdfe4_link_led_blink(i,hwdev);
				break;
			case MAIN_DATA_MODE:
				sdfe4_link_led_fast_blink(i,hwdev);
				break;
			case MAIN_INIT:
				sdfe4_link_led_down(i,hwdev);
				sdfe4_clock_setup(i,hwdev);
				sdfe4_reset_hwdev_chan(&(hwdev->ch[i]));
				if( sdfe4_setup_channel(i,hwdev) ){
					PDEBUG(debug_error,"error in sdfe4_setup_channel");
					// Retry at next mon interval
					chan->state_change=1;	// ATOMIC ?
					continue;
				}
				if( sdfe4_start_channel(i,hwdev) ){
					PDEBUG(debug_error,"error in sdfe4_start_channel");
					// Retry at next mon interval
					chan->state_change=1;	// ATOMIC ?
					continue;
				}
				wait_ms(10);
				break;
			case MAIN_EXCEPTION:
			  	break;
			}
		}
			
		if( chan->conn_state_change ){
			switch( chan->conn_state ){
			case GHS_STARTUP:
				break;
			case GHS_TRANSFER:
				break;
			case EXCEPTION:
				break;
			case GHS_30SEC_TIMEOUT:
				// SRU specific
			  	break;
			}
			chan->conn_state_change=0;
		}

		if(chan->sdi_dpll_sync){
			chan->sdi_dpll_sync = 0;
			sdfe4_load_config(i,hwdev);
			sdfe4_link_led_up(i,hwdev);
		}

		if(cfg->need_reconf){
			cfg->need_reconf = 0;

			sdfe4_disable_channel(i,hwdev);
			sdfe4_link_led_down(i,hwdev);
			sdfe4_clock_setup(i,hwdev);
			sdfe4_reset_hwdev_chan(&(hwdev->ch[i]));			
			wait_ms(200);
			if( sdfe4_setup_channel(i,hwdev) ){
				PDEBUG(debug_error,"error in sdfe4_setup_channel");
				return -1;
			}
			if( sdfe4_start_channel(i,hwdev) ){
				PDEBUG(debug_error,"error in sdfe4_start_channel");
				return -1;
			}
		}
	}
	return 0;
}

/*----------------------------------------------------------
  EOC Functions
  -----------------------------------------------------------*/

/*
 * sdfe4_eoc_init:
 * 	initialise new arrived EOC message and save it to 
 *	channels (ch) internal buffer for future process
 *	by OS applications
 *	Parameters:
 *	@ch - channel reseived message
 *	@ptr - pointer to message
 *	@size - message size
 */
void sdfe4_eoc_init(struct sdfe4_channel *ch,char *ptr,int size)
{
	int eflag;
	int msize;
	PDEBUG(debug_eoc,"ptr=%p, size=%d,msg_size=%d",ptr,size,ptr[2]);

	// check if size is valid    
	if( size < 4 ){
		PDEBUG(debug_error,": abort - small size");
		return;
	}

	if( ptr[1] ){
		// receiption failed
		eoc_abort_new(ch->eoc);
		PDEBUG(debug_error,": abort - failed receiption");		
		return;
	}
	    
	eflag = ptr[0]; // message completion flag
	msize = ptr[2]; // message length
	PDEBUG(debug_eoc,"eflag=%d,msize=%d",eflag,msize);	
	/*	if( msize != (size-3) ){
	// bad block size
	PDEBUG(debug_eoc,"Abort: sizes mismatch");	
	eoc_abort_cur(ch->eoc);
	return;
	}
	*/	PDEBUG(debug_eoc,"process appending");	
	eoc_append(ch->eoc,&ptr[3],msize,eflag);
}		

/*
 * sdfe4_eoc_init:
 * 	initialise new arrived EOC message and save it to 
 *	channels (ch) internal buffer for future process
 *	by OS applications
 *	Parameters:
 *	@ch - channel reseived message
 *	@ptr - pointer to message
 *	@size - message size
 */
inline int sdfe4_eoc_rx(struct sdfe4_channel *ch,char **ptr){
	return eoc_get_cur(ch->eoc,ptr);
}

int
sdfe4_eoc_tx(struct sdfe4 *hwdev,int ch,char *ptr,int size)
{
	int count = size/SDFE4_EOC_MAXMSG + 1;
	char msg[SDFE4_EOC_MAXMSG+2];
	struct sdfe4_msg rmsg;	
	int i,offset = 0;
	u8 cp;
	int ret;

	for(i=0;i<count;i++){
		cp = (size>SDFE4_EOC_MAXMSG) ? SDFE4_EOC_MAXMSG : size;
		memcpy(msg+2,ptr+offset,cp);
		offset += cp;
		size -= cp;
		rmsg.ack_id=ACK_EOC_TX;
		PDEBUG(debug_eoc,"Call CMD_EOC_TX, send %d bytes",cp);
		hwdev->ch[ch].eoc->eoc_tx = 0;
		if( i+1 == count ){
			msg[0] = 1;
		}else{
			msg[0] = 0;
		}
		msg[1] = cp;
		hwdev->ch[ch].eoc->eoc_tx = 0;
		if(sdfe4_pamdsl_cmd(ch,CMD_EOC_TX,(u8*)msg,cp+2,&rmsg,hwdev)){
			PDEBUG(debug_error,"CMD_EOC_TX failed");		
			return -1;
		}
		PDEBUG(debug_eoc,"CMD_EOC_TX: success");
		PDEBUG(debug_eoc,"wait for NFC");
		ret = sdfe4_eoc_wait_intr(1500,hwdev);
		if( i+1==count ){
			if( hwdev->ch[ch].eoc->eoc_tx != EOC_TX_READY ){
				PDEBUG(debug_error,"Fail to trasnmit, no EOC_TX_READY, get %x, ret=%d",
						hwdev->ch[ch].eoc->eoc_tx,ret);
				return -1;
			}
			PDEBUG(debug_eoc,"TX Successful");			
		}else if( hwdev->ch[ch].eoc->eoc_tx != EOC_TX_FRAME ){
			PDEBUG(debug_error,"Fail to trasnmit, no EOC_TX_FRAME, get %x, ret=%d",
				hwdev->ch[ch].eoc->eoc_tx,ret);
			return -1;
		}
	
		PDEBUG(debug_eoc,"Interation %d success",i);
	}
	PDEBUG(debug_eoc,"TX success");	
	return 0;
}
