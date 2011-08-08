/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home/cvsroot/uClinux-dist/loader_srcroot/inc/rtl8650/vlanTable.h,v 1.2 2004/03/31 01:49:20 yjlou Exp $
*
* Abstract: Switch core vlan table access header file.
*
* $Author: yjlou $
*
* $Log: vlanTable.h,v $
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

#ifndef _VLANTABLE_H_
#define _VLANTABLE_H_



/* VLAN table access routines 
*/

/* Create vlan 
Return: EEXIST- Speicified vlan already exists.
        ENFILE- Destined slot occupied by another vlan.*/
int32 vlanTable_create(uint32 vid, rtl_vlan_param_t * param);

/* Destroy vlan 
Return: ENOENT- Specified vlan id does not exist.*/
int32 vlanTable_destroy(uint32 vid);

/* Add a member port
Return: ENOENT- Specified vlan id does not exist.*/
int32 vlanTable_addMemberPort(uint32 vid, uint32 portNum);

/* Remove a member port 
Return: ENOENT- Specified vlan id does not exist.*/
int32 vlanTable_removeMemberPort(uint32 vid, uint32 portNum);

/* Set a member port list 
Return: ENOENT- Specified vlan id does not exist.*/
int32 vlanTable_setMemberPort(uint32 vid, uint32 portList);

/* Set ACL rule 
Return: ENOENT- Specified vlan id does not exist.*/
int32 vlanTable_setAclRule(uint32 vid, uint32 inACLStart, uint32 inACLEnd,
                                uint32 outACLStart, uint32 outACLEnd);

/* Get ACL rule 
Return: ENOENT- Specified vlan id does not exist.*/
int32 vlanTable_getAclRule(uint32 vid, uint32 *inACLStart_P, uint32 *inACLEnd_P,
                                uint32 *outACLStart_P, uint32 *outACLEnd_P);

/* Set vlan as internal interface 
Return: ENOENT- Specified vlan id does not exist.*/
int32 vlanTable_setInternal(uint32 vid);

/* Set vlan as external interface 
Return: ENOENT- Specified vlan id does not exist.*/
int32 vlanTable_setExternal(uint32 vid);

/* Enable hardware routing for this vlan 
Return: ENOENT- Specified vlan id does not exist.*/
int32 vlanTable_enableHardwareRouting(uint32 vid);

/* Disable hardware routing for this vlan 
Return: ENOENT- Specified vlan id does not exist.*/
int32 vlanTable_disableHardwareRouting(uint32 vid);

/* Set spanning tree status 
Return: ENOENT- Specified vlan id does not exist.*/
int32 vlanTable_setPortStpStatus(uint32 vid, uint32 portNum, uint32 STPStatus);

/* Get spanning tree status 
Return: ENOENT- Specified vlan id does not exist.*/
int32 vlanTable_getPortStpStatus(uint32 vid, uint32 portNum, uint32 *STPStatus_P);

/* Set spanning tree status 
Return: ENOENT- Specified vlan id does not exist.*/
int32 vlanTable_setStpStatus(uint32 vid, uint32 STPStatus);

/* Get information 
Return: ENOENT- Specified vlan id does not exist.*/
int32 vlanTable_getInformation(uint32 vid, rtl_vlan_param_t * param_P);

/* Get hardware information 
Return: ENOENT- Specified vlan id does not exist.*/
int32 vlanTable_getHwInformation(uint32 vid, rtl_vlan_param_t * param_P);

/* Get vlan id 
Return: ENOENT- Specified slot does not exist.*/
int32 vlanTable_getVidByIndex(uint32 eidx, uint32 * vid_P);



/* Hardware bit allocation of VLAN table 
*/
typedef struct {
    /* word 0 */
    uint16          mac31_16;
    uint16          mac15_0;
    
    /* word 1 */
    uint16          vhid        : 9;
    uint16          memberPort  : 6;
    uint16          valid       : 1;
    uint16          mac47_32;
    /* word 2 */
    uint8           reserv5     : 1;
    uint8           outACLEnd   : 7;
    uint8           reserv4     : 1;
    uint8           outACLStart : 7;
    uint8           reserv3     : 1;
    uint8           inACLEnd    : 7;
    uint8           reserv2     : 1;
    uint8           inACLStart  : 7;
    /* word 3 */
    uint32          mtuL        : 8;
    uint32          macMask     : 2;
    uint32          egressUntag : 6;
    uint32          promiscuous : 1;
    uint32          bcastToCPU  : 1;
    uint32          STPStatus   : 12;
    uint32          enHWRoute   : 1;
    uint32          isInternal  : 1;
    /* word 4 */
    uint32          reserv7     : 29;
    uint32          mtuH        : 3;
} vlan_table_t;



#endif /*_VLANTABLE_H_*/
