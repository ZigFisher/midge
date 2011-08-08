
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

#include "rtl_types.h"
#include "tftpnaive.h"
#include "net.h"
#include "swNic_poll.h"

NetInterface netif;

static BootData bdata;
BootData *tftp_req;


#if 0
unsigned char packetBuf[MAXPACKETBUF][MAXPACKETLEN];
unsigned short packetBufLen[MAXPACKETBUF];
volatile int idxiPBuf = 0, idxoPBuf = 0;
#endif

void processPacket (void)
{
#if 0
	register int idxo = idxoPBuf;
	register EthHeader *hdr;

	if (idxo != idxiPBuf) {
		idxo = (++idxo) % MAXPACKETBUF;
		hdr = (EthHeader *) packetBuf[idxo];
		if (hdr->frametype == FRAME_ARP) {
			processARP (packetBuf[idxo], packetBufLen[idxo]);
		}
		else if (hdr->frametype == FRAME_IP) {
			processIP (packetBuf[idxo], packetBufLen[idxo]);
		}
		idxoPBuf = idxo;
		++netif.RxPacket;
	}
#endif
    void * pPkt;
    uint32 len;
    EthHeader * hdr;
    
    if ( swNic_receive(&pPkt, &len) == 0 )
    {
        hdr = (EthHeader *) pPkt;
        if (hdr->frametype == FRAME_ARP)
            processARP (pPkt, len);
        else if (hdr->frametype == FRAME_IP)
            processIP (pPkt, len);
    }
}

#if 0
void netStat (void)
{
	printf ("Total IRQ   : %d\n", netif.TotalIRQ);
	printf ("Total event : %d\n", netif.TotalEvent);
	printf ("Tx event    : %d\n", netif.TxEvent);
	printf ("Rx event    : %d\n", netif.RxEvent);
	printf ("Buf event   : %d\n", netif.BufEvent);
	printf ("Rx missed   : %d\n", netif.RxMiss);
	printf ("Rx dropped  : %d\n", netif.RxQFull);
	printf ("Collisions  : %d\n", netif.TxColl);
	printf ("Unknown ev  : %d\n", netif.UnknownEvent);
	printf ("Rx Packets  : %d\n", netif.RxPacket);
}
#endif

unsigned short cks_partial (void *_buf, unsigned short len, long sum)
{
	unsigned short *buf = _buf;

	while (len > 1) {
		sum += *buf++;
		if (sum & 0x80000000)
			sum = (sum & 0xffff) + (sum >> 16);
		len -= 2;
	}
	if (len)
		sum += (unsigned short) *(unsigned char *) buf;
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return sum;
}

unsigned short cks (void *buf, unsigned short len)
{
	return ~cks_partial (buf, len, 0);
}

/* decode the filename passed as argument to 
 * retrieve the download address (p=0) or the
 * start offset (p=1).
 * the filename is as follow:
 *   name.<downld-addr>.<start-offset>.ext
 * where <downld-addr> and <start-offset> are hexa numbers
 * i.e. de2.01e00000.400.bin
 */
#ifdef FAT_CODE
static unsigned int decode_filename (const char *f, int p)
{
	unsigned int ret = 0;
	const char *s = strchr (f, '.');

	/* if the starting address offset is asked for */
	if (p)
		s = strchr (s + 1, '.');

	if (s++) {
		while ((*s >= '0' && *s <= '9')
			   || ((*s & (~32)) >= 'A' && ((*s & (~32)) <= 'F'))) {
			ret <<= 4;
			if (*s >= '0' && *s <= '9') {
				ret += *s++ - '0';
			}
			else {
				ret += ((*s++) & (~32)) - 'A' + 10;
			}
		}
	}
	return ret;
}
#endif//FAT_CODE

/* request an IP address then check for conflict
 */
static void getIP (int wait)
{
	netif.ip = 0;

	/* send the BOOTP request and wait for reply */
	if (sendBOOTPRequest () == 0)
		printf ("BOOTP request sent\n");
	busyWait (wait, processPacket, &netif.ip);

	/* check for IP address conflict */
	if (netCheckIP()) {
		/* this is to make sure we are sending BOOTP request
		 * with the delay specified.
		 */
		busyWait (wait, processPacket, 0);
	}
}

int netCheckIP (void)
{
	/* if we have our IP address, send
	 * a gratuituous ARP to check for IP conflict.
	 */
	if (netif.ip) {
		sendARPRequest (0, netif.ip);
		busyWait (250, processPacket, 0);
		if (netif.ip) {
			sendARPRequest (0, netif.ip);
			busyWait (250, processPacket, 0);
			if (netif.ip) {
				sendARPRequest (netif.ip, netif.ip);
				busyWait (250, processPacket, 0);
			}
		}
	}
	return (netif.ip == 0);
}

int netRequestIP (void)
{
	/* start a BOOTP request and wait for a reply
	 * if there is no reply after 1 sec, do a new
	 * BOOTP request and wait 2 sec, then again
	 * with a 4 sec wait, a 8 sec and start over
	 */
	getIP (1000);
	if (netif.ip == 0) {
		getIP (2000);
		if (netif.ip == 0) {
			getIP (4000);
			while (netif.ip == 0) {
				getIP (8000);
			}
		}
	}

	return (netif.ip == 0);
}

int netTFTPGet (void)
{
	int lastbloc;

	if (netif.ip && tftp_req) {
		tftp_req->bcnt = tftp_req->sts = tftp_req->port = 0;
		lastbloc = tftp_req->bloc = 1;
		if (sendTFTPRequest () == 0) {
			printf ("TFTP request: ");
			printf ("server %d.%d.%d.%d, ",
					(tftp_req->server >> 24) & 0xff,
					(tftp_req->server >> 16) & 0xff,
					(tftp_req->server >> 8) & 0xff,
					tftp_req->server & 0xff);
			printf ("file '%s' ", tftp_req->file);
			printf ("addr 0x%08x ", tftp_req->addr);
			if (tftp_req->strtf)
				printf ("start 0x%08x", tftp_req->strt);
			printf ("\n");
			while (1) {
				//busyWait (3000, processPacket, &tftp_req->sts);
				{
				while (/*countdown-- &&*/ !tftp_req->sts)
				    processPacket();
				}
				if (tftp_req->sts > 0)
					return 0;
				if (tftp_req->sts < 0)
					return 1;
				if (lastbloc == tftp_req->bcnt)
					break;
				lastbloc = tftp_req->bcnt;
			}
			printf ("TFTP: receive timeout\n");
		}
	}
	return 1;
}

/* this is the entry point function
 * to initiate the network boot
 */
int bootpReceive (char * pMac, int * pSize, void * bufBase)
{
	int reqtry;
	
	memcpy(netif.IEEEIA, pMac, 6);
	netif.send = swNic_send;
	
	memset (&bdata, 0, sizeof bdata);
	tftp_req = &bdata;

	/* request an IP address through BOOTP */
	netRequestIP ();
	
	tftp_req->addr = (unsigned int) bufBase;
	tftp_req->strt = 0;

	/* if the IP is still valid (no conflict)
	 * and there is a valid TFTP request to be started
	 * try 5 times to download
	 */
	if (netif.ip && tftp_req) {
		tftp_req->strtf = tftp_req->inited = 1;
		for (reqtry = 5; reqtry && netif.ip; --reqtry) {
			if (netTFTPGet () == 0)
			{
			  *pSize = tftp_req->bcnt;
				return 0;
			}
		}
	}

	return 1;
}

#if TFTPNAIVE_TIMEREQUEST

int timestamp;

int netTimestampRequest(int ip)
{
	timestamp = 0;
	if (netif.ip && ip)
		if (sendICMPTimestampRequest (ip) == 0)
			busyWait (500, processPacket, &timestamp);
	return timestamp;
}

#endif
