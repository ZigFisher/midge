/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home/cvsroot/uClinux-dist/loader_srcroot/src/rtl8650/vlanTable.c,v 1.3 2005/01/28 02:03:48 yjlou Exp $
*
* Abstract: Switch core vlan table access driver source code.
*
* $Author: yjlou $
*
* $Log: vlanTable.c,v $
* Revision 1.3  2005/01/28 02:03:48  yjlou
* *: loader version migrates to "00.00.19".
* +: support Hub mode
* +: Ping mode support input IP address
* *: clear WDTIND always.
*
* Revision 1.2  2004/03/31 01:49:20  yjlou
* *: all text files are converted to UNIX format.
*
* Revision 1.1  2004/03/16 06:36:13  yjlou
* *** empty log message ***
*
* Revision 1.2  2004/03/09 00:46:12  danwu
* remove unused code to shrink loader image size under 0xc000 and only flash block 0 & 3 occupied
*
* Revision 1.1.1.1  2003/09/25 08:16:55  tony
*  initial loader tree 
*
* Revision 1.1.1.1  2003/05/07 08:16:06  danwu
* no message
*
* ---------------------------------------------------------------
*/

#include <rtl_types.h>
#include <rtl_errno.h>
#include <rtl8650/asicregs.h>
#include <rtl8650/swCore.h>
#include <rtl8650/vlanTable.h>



/* STATIC VARIABLE DECLARATIONS
 */



/* LOCAL SUBPROGRAM SPECIFICATIONS
 */



int32 vlanTable_create(uint32 vid, rtl_vlan_param_t * param)
{
    vlan_table_t    entryContent;
    
    ASSERT_CSP(param);
    
    swTable_readEntry(TYPE_VLAN_TABLE, vid & 7, &entryContent);
    
    if ( entryContent.valid )
    {
        if ( entryContent.vhid == (vid >> 3) )
            /* Specified vlan id already exists */
            return EEXIST;
        else
            return ENFILE;
    }

	bzero( (void *) &entryContent, sizeof(entryContent) );
    entryContent.mac47_32 = param->gMac.mac47_32;
    entryContent.mac31_16 = param->gMac.mac31_16;
    entryContent.mac15_0 = param->gMac.mac15_0;
    entryContent.valid = 1;
    entryContent.memberPort = param->memberPort & ALL_PORT_MASK;
    entryContent.vhid = vid >> 3;
    entryContent.promiscuous = param->promiscuous;
    entryContent.bcastToCPU = param->bcastToCPU;
    entryContent.egressUntag = param->egressUntag;
    entryContent.mtuH = param->mtu >> 8;
    entryContent.mtuL = param->mtu & 0xff;
    entryContent.outACLStart = 0;
    entryContent.outACLEnd = 0;
    entryContent.inACLStart = 0;
    entryContent.inACLEnd = 0;
    entryContent.isInternal = 1;
    
    /* Write into hardware */
    if ( swTable_addEntry(TYPE_VLAN_TABLE, vid & 7, &entryContent) == 0 )
        return 0;
    else
        /* There might be something wrong */
        ASSERT_CSP( 0 );
}

int32 vlanTable_destroy(uint32 vid)
{
    vlan_table_t    entryContent;
    
    swTable_readEntry(TYPE_VLAN_TABLE, vid & 7, &entryContent);
    
    if ( !entryContent.valid || 
            (entryContent.vhid != (vid >> 3)) )
        /* Specified vlan id does not exist */
        return ENOENT;
    
    bzero(&entryContent, sizeof(vlan_table_t));
    
    /* Write into hardware */
    if ( swTable_modifyEntry(TYPE_VLAN_TABLE, vid & 7, &entryContent) == 0 )
        return 0;
    else
        /* There might be something wrong */
        ASSERT_CSP( 0 );
}

int32 vlanTable_setStpStatusOfAllPorts(uint32 vid, uint32 STPStatus)
{
    vlan_table_t    entryContent;
    uint32          portNum;
    uint32          val = STPStatus;
    
    ASSERT_CSP( STPStatus <= STP_FORWARD );
    
    swTable_readEntry(TYPE_VLAN_TABLE, vid & 7, &entryContent);
    
    if ( !entryContent.valid || 
            (entryContent.vhid != (vid >> 3)) )
        /* Specified vlan id does not exist */
        return ENOENT;
    
    for (portNum=1; portNum < MAX_PORT_NUMBER; portNum++)
        val |= (val << 2);
    entryContent.STPStatus = val;
    
    /* Write into hardware */
    if ( swTable_modifyEntry(TYPE_VLAN_TABLE, vid & 7, &entryContent) == 0 )
        return 0;
    else
        /* There might be something wrong */
        ASSERT_CSP( 0 );
}
