/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home/cvsroot/uClinux-dist/loader_srcroot/inc/rtl_dev.h,v 1.2 2004/03/31 01:49:20 yjlou Exp $
*
* Abstract: Device driver interface header file.
*
* $Author: yjlou $
*
* $Log: rtl_dev.h,v $
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

#ifndef _RTL_DEV_H_
#define _RTL_DEV_H_



/* --------------------------------------------------------------------
 * ROUTINE NAME - dev_init
 * --------------------------------------------------------------------
 * FUNCTION: This service is called by physical device driver or the 
        kernel to initialize the corresponding driver interface.
 * INPUT   :
		devId: Device ID.
		oflag: Operation flag specifying ability this device driver 
		    should support:
		    O_BLOCK: Blocking read.
		    O_NONBLOCK: Non-blocking read.
		    O_WRITECONF: Write confirm callback.
		    O_IOCTLCONF: I/O control confirm callback.
		phyDevRead: Physical device driver read routine.
		phyDevWrite: Physical device driver write routine.
		phyDevIoctl: Physical device driver I/O control routine.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns zero. 
        Otherwise, the function returns -1.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 dev_init(uint32 devId, 
        uint32 oflag, 
        int32 (*phyDevRead)(void * input), 
        int32 (*phyDevWrite)(void * output, void (*dev_dataOut)(void * output, uint32 sflag)), 
        int32 (*phyDevIoctl)(uint32 cmd, void * arg, void (*dev_ioctlDone)(uint32 cmd, void * arg, uint32 sflag)));


/* --------------------------------------------------------------------
 * ROUTINE NAME - dev_read
 * --------------------------------------------------------------------
 * FUNCTION: This service performs read from device.
 * INPUT   :
		devId: Device ID.
		oflag: Operation flag.
		    O_BLOCK: Blocking read.
		    O_NONBLOCK: Non-blocking read.
 * OUTPUT  : 
        input: Pointer to an area to hold input data.
 * RETURN  : Upon successful completion, the function returns zero. 
        Otherwise, the function returns 
            EAGAIN: No input data available, 
            EINVAL: Invalid oflag.
        or other values depending on which device.
 * NOTE    : The user might be blocked by this call, depending on the 
        specified oflag and the ability this device driver supports. 
        Reference dev_init.
 * -------------------------------------------------------------------*/
int32 dev_read(uint32 devId, uint32 oflag, void * input);


/* --------------------------------------------------------------------
 * ROUTINE NAME - dev_write
 * --------------------------------------------------------------------
 * FUNCTION: This service performs write to device.
 * INPUT   :
		devId: Device ID.
		output: Pointer to an area to hold output data.
		dev_writeConf: Write confirm callback routine. The driver 
		    interface will invoke this registered callback to inform 
		    status of write request. Specify NULL if no confirm is 
		    required or O_WRITECONF is not supported. Input parameters 
		    are: 
		        output: Pointer to an area to hold output data.
		        sflag: Status flag. Drivers may defines values for 
		            their own use with the default zero indicating 
		            complete without error.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns zero. 
        Otherwise, the function returns 
            EAGAIN: Data cannot be written tempararily, 
        or other values depending on which device.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 dev_write(uint32 devId, void * output, 
                void (*dev_writeConf)(void * output, uint32 sflag));


/* --------------------------------------------------------------------
 * ROUTINE NAME - dev_ioctl
 * --------------------------------------------------------------------
 * FUNCTION: This service performs I/O control to device.
 * INPUT   :
		devId: Device ID.
		cmd: Control command. Drivers may defines values for their own 
		    use.
		arg: Argument.
		dev_ioctlConf: I/O control confirm callback routine. The driver 
		    interface will invoke this registered callback to inform 
		    status of control request. Specify NULL if no confirm is 
		    required or O_WRITECONF is not supported. Input parameters 
		    are: 
        		cmd: Control command.
        		arg: Argument.
		        sflag: Status flag. Drivers may defines values for 
		            their own use with the default zero indicating 
		            complete without error.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns zero. 
        Otherwise, the function returns values depending on which 
        device.
 * NOTE    : Currently dev_ioctlConf is NOT supported.
 * -------------------------------------------------------------------*/
int32 dev_ioctl(uint32 devId, uint32 cmd, void * arg, 
                void (*dev_ioctlConf)(uint32 cmd, void * arg, uint32 sflag));


/* --------------------------------------------------------------------
 * ROUTINE NAME - dev_dataIn
 * --------------------------------------------------------------------
 * FUNCTION: This service is called by physical device driver (probably 
        rx interrupt service routine) to notify receiving data.
 * INPUT   :
		devId: Device ID.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : This function should be called if O_BLOCK is supported.
 * -------------------------------------------------------------------*/
void dev_dataIn(uint32 devId);


void dev_kernelInit(void);


/* Device ID Definitions 
*/
#define UART_DEVID       0
#define FLASH_DEVID      1
#define SWNIC_DEVID     2
#define DRVNIC_DEVID    3
#define MAX_DEVID        16



/* Operation Flag Definitions 
*/
#define O_BLOCK         (1 << 0)
#define O_NONBLOCK      (1 << 1)
#define O_WRITECONF     (1 << 2)
#define O_IOCTLCONF     (1 << 3)
#define O_VALID         (1 << 30)



#endif /*_RTL_DEV_H_*/
