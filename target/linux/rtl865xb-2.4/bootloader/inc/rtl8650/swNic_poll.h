/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home/cvsroot/uClinux-dist/loader_srcroot/inc/rtl8650/swNic_poll.h,v 1.2 2004/03/31 01:49:20 yjlou Exp $
*
* Abstract: Switch core polling mode NIC header file.
*
* $Author: yjlou $
*
* $Log: swNic_poll.h,v $
* Revision 1.2  2004/03/31 01:49:20  yjlou
* *: all text files are converted to UNIX format.
*
* Revision 1.1  2004/03/16 06:36:13  yjlou
* *** empty log message ***
*
* Revision 1.1.1.1  2003/09/25 08:16:56  tony
*  initial loader tree 
*
* Revision 1.1.1.1  2003/05/07 08:16:07  danwu
* no message
*
* ---------------------------------------------------------------
*/


#ifndef _SWNIC_H
#define _SWNIC_H


/* --------------------------------------------------------------------
 * ROUTINE NAME - swNic_init
 * --------------------------------------------------------------------
 * FUNCTION: This service initializes the switch NIC.
 * INPUT   : 
        nRxPkthdrDesc: Number of Rx pkthdr descriptors.
        nRxMbufDesc: Number of Rx mbuf descriptors.
        nTxPkthdrDesc: Number of Tx pkthdr descriptors.
        clusterSize: Size of a mbuf cluster.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns ENOERR. 
        Otherwise, 
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swNic_init(uint32 nRxPkthdrDesc, uint32 nRxMbufDesc, uint32 nTxPkthdrDesc, 
                        uint32 clusterSize);

/* --------------------------------------------------------------------
 * ROUTINE NAME - swNic_intHandler
 * --------------------------------------------------------------------
 * FUNCTION: This function is the NIC interrupt handler.
 * INPUT   :
		intPending: Pending interrupts.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
void swNic_intHandler(uint32 intPending);

int32 swNic_receive(void** input, uint32* pLen);
int32 swNic_send(void * output, uint32 len);

#endif /* _SWNIC_H */
