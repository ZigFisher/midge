
/*
	Copyright 2001, 2002 Georges Menie (www.menie.org)

	This file is part of Tftpnaive.

    Tftpnaive is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Tftpnaive is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Tftpnaive; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _TFTPNAIVE_H_
#define _TFTPNAIVE_H_

#include "rtl_types.h"

typedef struct {
	unsigned int ip;
	int TotalIRQ;
	int TotalEvent;
	int UnknownEvent;
	int RxEvent;
	int TxEvent;
	int BufEvent;
	int RxMiss;
	int TxColl;
	int RxQFull;
	int RxPacket;
	int32 (*send) (void *, uint32);
	unsigned char IEEEIA[6];
} NetInterface;

typedef struct {
	unsigned int server;
	unsigned int port;
	unsigned short bloc;
	unsigned short strtf;
	unsigned int strt;
	unsigned int addr;
	unsigned int bcnt;
	unsigned int inited;
	unsigned int sts;
	char file[128];
} BootData;

extern NetInterface netif;
extern BootData *tftp_req;

extern int netUp (void);
extern int netRequestIP (void);
extern int netCheckIP (void);
extern int netBoot (void);
extern int netTFTPGet (void);
extern int netDown (void);

#if TFTPNAIVE_TIMEREQUEST
extern int netTimestampRequest (int);
#endif

/*-------------*/
/* Error codes */
/*-------------*/

#define ERR_LINK	1
#define ERR_CHIPID	2
#define ERR_TIMEOUT	3

/* Prevent compiler from generating internal sub-routine call code at the 
   function prologue and epilogue automatically, since the inline code 
   causes gdb failure */
#pragma ghs inlineprologue

#endif
