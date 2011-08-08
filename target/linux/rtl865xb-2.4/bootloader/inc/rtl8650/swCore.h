/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home/cvsroot/uClinux-dist/loader_srcroot/inc/rtl8650/swCore.h,v 1.3 2004/03/31 01:49:20 yjlou Exp $
*
* Abstract: Switch core header file.
*
* $Author: yjlou $
*
* $Log: swCore.h,v $
* Revision 1.3  2004/03/31 01:49:20  yjlou
* *: all text files are converted to UNIX format.
*
* Revision 1.2  2004/03/30 11:34:38  yjlou
* *: commit for 80B IC back
*   +: system clock rate definitions have changed.
*   *: fixed the bug of BIST_READY_PATTERN
*   +: clean all ASIC table when init ASIC.
* -: define _L2_MODE_ to support L2 switch mode.
*
* Revision 1.1  2004/03/16 06:36:13  yjlou
* *** empty log message ***
*
* Revision 1.2  2004/03/16 06:04:04  yjlou
* +: support pure L2 switch mode (for hardware testing)
*
* Revision 1.1.1.1  2003/09/25 08:16:56  tony
*  initial loader tree 
*
* Revision 1.1.1.1  2003/05/07 08:16:07  danwu
* no message
*
* ---------------------------------------------------------------
*/

#ifndef _SWCORE_H
#define _SWCORE_H



#define SWNIC_DEBUG
#define SWTABLE_DEBUG
#define SWCORE_DEBUG



/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_init
 * --------------------------------------------------------------------
 * FUNCTION: This service initializes the switch core.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENFILE: Destination slot of vlan table is occupied.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_init(void);
#ifdef _L2_MODE_
int32 L2_swCore_config( uint8* gmac, uint32 for_RTL8650A );
#endif//_L2_MODE_


/* VLAN service
*/

#define RTL_STP_DISABLE 0
#define RTL_STP_BLOCK   1
#define RTL_STP_LEARN   2
#define RTL_STP_FORWARD 3

/* VLAN access parameters */
typedef struct {
    uint32          memberPort;
    uint32          egressUntag;
    macaddr_t       gMac;
    uint16          bcastToCPU  : 1;
    uint16          promiscuous : 1;
    uint16          reserv0     : 14;
    uint32          mtu;
} rtl_vlan_param_t;


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanCreate
 * --------------------------------------------------------------------
 * FUNCTION: This service creates a vlan.
 * INPUT   :
		param_P: Pointer to the parameters.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
    EEXIST: Speicified vlan already exists.
		ENFILE: Destination slot of vlan table is occupied.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanCreate(uint32 vid, rtl_vlan_param_t * param_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanDestroy
 * --------------------------------------------------------------------
 * FUNCTION: This service destroys a vlan.
 * INPUT   :
		vid: Vlan ID.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanDestroy(uint32 vid);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanSetPVid
 * --------------------------------------------------------------------
 * FUNCTION: This service sets port based vlan id.
 * INPUT   :
		portNum: Port number.
		pvid: Vlan ID.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanSetPVid(uint32 portNum, uint32 pvid);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanGetPVid
 * --------------------------------------------------------------------
 * FUNCTION: This service gets port based vlan id.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : 
		pvid_P: Pointer to a variable to hold the PVid.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanGetPVid(uint32 portNum, uint32 *pvid_P);

/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanSetPortSTPStatus
 * --------------------------------------------------------------------
 * FUNCTION: This service sets the spanning tree status of the 
        specified port.
 * INPUT   :
		vid: Vlan ID.
		portNum: Port number.
		STPStatus: Spanning tree status. Valid values are RTL_STP_DISABLE, 
		        RTL_STP_BLOCK, RTL_STP_LEARN and RTL_STP_FORWARD.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanSetPortSTPStatus(uint32 vid, uint32 portNumber, uint32 STPStatus);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanSetSTPStatusOfAllPorts
 * --------------------------------------------------------------------
 * FUNCTION: This service sets the spanning tree status.
 * INPUT   :
		vid: Vlan ID.
		STPStatus: Spanning tree status. Valid values are RTL_STP_DISABLE, 
		        RTL_STP_BLOCK, RTL_STP_LEARN and RTL_STP_FORWARD.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanSetSTPStatusOfAllPorts(uint32 vid, uint32 STPStatus);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanGetPortSTPStatus
 * --------------------------------------------------------------------
 * FUNCTION: This service gets the spanning tree status of the 
        specified port.
 * INPUT   :
		vid: Vlan ID.
		portNum: Port number.
 * OUTPUT  : 
		STPStatus_P: Pointer to a variable to hold the spanning tree 
		        status of the specified port.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanGetPortSTPStatus(uint32 vid, uint32 portNumber, uint32 *STPStatus_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_vlanGetInformation
 * --------------------------------------------------------------------
 * FUNCTION: This service gets information of the specified vlan.
 * INPUT   :
		vid: Vlan ID.
 * OUTPUT  : 
		param_P: Pointer to an area to hold the parameters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified vlan does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_vlanGetInformation(uint32 vid, rtl_vlan_param_t * param_P);


/* Layer 2 service
*/

/* L2 forwarding table access parameters 
*/
typedef struct {
    macaddr_t       mac;
    uint16          isStatic    : 1;
    uint16          hPriority   : 1;
    uint16          toCPU       : 1;
    uint16          srcBlock    : 1;
    uint16          nxtHostFlag : 1;
    uint16          reserv0     : 11;
    uint32          memberPort;
    uint32          agingTime;
} rtl_l2_param_t;


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_staticMacAddrAdd
 * --------------------------------------------------------------------
 * FUNCTION: This service adds the static MAC address.
 * INPUT   :
		param_P: Pointer to the parameters.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		ENFILE: Cannot allocate slot.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_staticMacAddrAdd(rtl_l2_param_t * param_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_staticMacAddrRemove
 * --------------------------------------------------------------------
 * FUNCTION: This service removes the specified static MAC address.
 * INPUT   :
		param_P: Pointer to the parameters.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise, 
		ENOENT: Specified MAC address does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_staticMacAddrRemove(rtl_l2_param_t * param_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_layer2TableGetInformation
 * --------------------------------------------------------------------
 * FUNCTION: This service gets information of specified L2 switch table 
        entry.
 * INPUT   :
        entryIndex: Index of entry.
 * OUTPUT  : 
		param_P: Pointer to an area to hold the parameters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
        EEMPTY: Specified entry is empty.
		ENOENT: Specified entry does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_layer2TableGetInformation(uint32 entryIndex, rtl_l2_param_t * param_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_layer2TableGetInformationByMac
 * --------------------------------------------------------------------
 * FUNCTION: This service gets information of specified L2 switch table 
        entry.
 * INPUT   : None.
 * OUTPUT  : 
		param_P: Pointer to an area to hold the parameters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		ENOENT: Specified entry does not exist.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_layer2TableGetInformationByMac(rtl_l2_param_t * param_P);


/* Counter service 
*/

typedef struct {
    uint32  etherStatsOctets;
    uint32  etherStatsDropEvents;
    uint32  etherStatsCRCAlignErrors;
    uint32  etherStatsFragments;
    uint32  etherStatsJabbers;
    uint32  ifInUcastPkts;
    uint32  etherStatsMulticastPkts;
    uint32  etherStatsBroadcastPkts;
    uint32  etherStatsUndersizePkts;
    uint32  etherStatsPkts64Octets;
    uint32  etherStatsPkts65to127Octets;
    uint32  etherStatsPkts128to255Octets;
    uint32  etherStatsPkts256to511Octets;
    uint32  etherStatsPkts512to1023Octets;
    uint32  etherStatsPkts1024to1518Octets;
    uint32  etherStatsOversizepkts;
    uint32  dot3ControlInUnknownOpcodes;
    uint32  dot3InPauseFrames;
} rtl_ingress_counter_t;
typedef struct {
    uint32  ifOutOctets;
    uint32  ifOutUcastPkts;
    uint32  ifOutMulticastPkts;
    uint32  ifOutBroadcastPkts;
    uint32  dot3StatsLateCollisions;
    uint32  dot3StatsDeferredTransmissions;
    uint32  etherStatsCollisions;
    uint32  dot3StatsMultipleCollisionFrames;
    uint32  dot3OutPauseFrames;
} rtl_egress_counter_t;

/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_counterGetMemberPort
 * --------------------------------------------------------------------
 * FUNCTION: This service gets all the member for counting.
 * INPUT   : None.
 * OUTPUT  : 
        portList_P: List of member ports.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_counterGetMemberPort(uint32 *portList_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_counterSetMemberPort
 * --------------------------------------------------------------------
 * FUNCTION: This service gets all the member for counting.
 * INPUT   : 
        portList: List of member ports.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_counterSetMemberPort(uint32 portList);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_counterGetIngress
 * --------------------------------------------------------------------
 * FUNCTION: This service gets all the ingress counters.
 * INPUT   : None.
 * OUTPUT  : 
        counters_P: Pointer to an area to hold the ingress counters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_counterGetIngress(rtl_ingress_counter_t *counters_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_counterGetEgress
 * --------------------------------------------------------------------
 * FUNCTION: This service gets all the egress counters.
 * INPUT   : None.
 * OUTPUT  : 
        counters_P: Pointer to an area to hold the egress counters.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_counterGetEgress(rtl_egress_counter_t *counters_P);


/* Port service 
*/

#define RTL_PORT_100M_FD        (1 << 0)
#define RTL_PORT_100M_HD        (1 << 1)
#define RTL_PORT_10M_FD         (1 << 2)
#define RTL_PORT_10M_HD         (1 << 3)

typedef struct {
    uint8   capableFlowCtrl : 1;
    uint8   capable100MFull : 1;
    uint8   capable100MHalf : 1;
    uint8   capable10MFull  : 1;
    uint8   capable10MHalf  : 1;
    uint8   reserv0         : 3;
} rtl_auto_nego_ability_t;
    
typedef struct {
    uint8   enAutoNego          : 1;
    uint8   enSpeed100M         : 1;
    uint8   enFullDuplex        : 1;
    uint8   enLoopback          : 1;
    uint8   linkEstablished     : 1;
    uint8   autoNegoCompleted   : 1;
    uint8   remoteFault         : 1;
    uint8   reserv0             : 1;
    rtl_auto_nego_ability_t   autoNegoAbility;
    rtl_auto_nego_ability_t   linkPartnerAutoNegoAbility;
    uint32  speedDuplex;
} rtl_port_status_t;

/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portSetSpeedDuplex
 * --------------------------------------------------------------------
 * FUNCTION: This service sets speed and duplex mode of specified port.
 * INPUT   :
		portNum: Port number.
		speedDuplex: Speed and duplex mode. Valid values are 
		    RTL_PORT_100M_FD, RTL_PORT_100M_HD, RTL_PORT_10M_FD and 
		    RTL_PORT_10M_HD.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portSetSpeedDuplex(uint32 portNum, uint32 speedDuplex);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portSetAutoNegociationAbility
 * --------------------------------------------------------------------
 * FUNCTION: This service sets auto negociation pause, speed and duplex 
        mode capability of specified port.
 * INPUT   :
		portNum: Port number.
		anAbility_P: Pointer to the data structure which specifies the auto 
		    negociation abilities.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portSetAutoNegociationAbility(uint32 portNum, rtl_auto_nego_ability_t *anAbility_P);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portEnableAutoNegociation
 * --------------------------------------------------------------------
 * FUNCTION: This service enables auto negociation of specified port.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portEnableAutoNegociation(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portDisableAutoNegociation
 * --------------------------------------------------------------------
 * FUNCTION: This service disables auto negociation of specified port.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portDisableAutoNegociation(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portRestartAutoNegociation
 * --------------------------------------------------------------------
 * FUNCTION: This service restarts auto negociation of specified port.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portRestartAutoNegociation(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portSetLoopback
 * --------------------------------------------------------------------
 * FUNCTION: This service sets specified port to loopback mode.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portSetLoopback(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portResetLoopback
 * --------------------------------------------------------------------
 * FUNCTION: This service sets specified port to normal mode.
 * INPUT   :
		portNum: Port number.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portResetLoopback(uint32 portNum);


/* --------------------------------------------------------------------
 * ROUTINE NAME - swCore_portGetStatus
 * --------------------------------------------------------------------
 * FUNCTION: This service gets port status of specified port.
 * INPUT   : 
		portNum: Port number.
 * OUTPUT  : 
    portStatus_P: Pointer to an area to hold the port status.
 * RETURN  : Upon successful completion, the function returns 0. 
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swCore_portGetStatus(uint32 portNum, rtl_port_status_t *portStatus_P);



#define swCore_vlanCreate vlanTable_create
#define swCore_vlanDestroy vlanTable_destroy
#define swCore_vlanSetPortSTPStatus vlanTable_setPortStpStatus
#define swCore_vlanSetSTPStatusOfAllPorts vlanTable_setStpStatusOfAllPorts
#define swCore_vlanGetPortSTPStatus vlanTable_getSTPStatus
#define swCore_vlanGetInformation vlanTable_getInformation

#endif /* _SWCORE_H */

