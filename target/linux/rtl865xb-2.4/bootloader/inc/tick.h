/*
 * Copyright c                Realtek Semiconductor Corporation, 2002
 * All rights reserved.                                                    
 * 
 * $Header: /home/cvsroot/uClinux-dist/loader_srcroot/inc/tick.h,v 1.1 2004/08/26 13:53:27 yjlou Exp $
 *
 * $Author: yjlou $
 *
 * Abstract:
 *
 *   Tick-related definition
 *
 * $Log: tick.h,v $
 * Revision 1.1  2004/08/26 13:53:27  yjlou
 * -: remove all warning messages!
 * +: add compile flags "-Wno-implicit -Werror" in Makefile to treat warning as error!
 *
 */

#ifndef _TICK_H_
#define _TICK_H_

void tick_pollStart(void);
uint32 tick_pollGet10MS(void);

#endif  /* _TICK_H_ */

