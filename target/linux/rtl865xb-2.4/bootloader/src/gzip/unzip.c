/* unzip.c -- decompress files in gzip or pkzip format. */


#ifdef RCSID
static char rcsid[] = "$Id: unzip.c,v 1.2 2004/03/31 01:49:20 yjlou Exp $";
#endif

#include "tailor.h"
#include "gzip.h"
#include "crypt.h"

/* PKZIP header definitions */
#define LOCSIG 0x04034b50L      /* four-byte lead-in (lsb first) */
#define LOCFLG 6                /* offset of bit flag */
#define  CRPFLG 1               /*  bit for encrypted entry */
#define  EXTFLG 8               /*  bit for extended local header */
#define LOCHOW 8                /* offset of compression method */
#define LOCTIM 10               /* file mod time (for decryption) */
#define LOCCRC 14               /* offset of crc */
#define LOCSIZ 18               /* offset of compressed size */
#define LOCLEN 22               /* offset of uncompressed length */
#define LOCFIL 26               /* offset of file name field length */
#define LOCEXT 28               /* offset of extra field length */
#define LOCHDR 30               /* size of local header, including sig */
#define EXTHDR 16               /* size of extended local header, inc sig */


/* Globals */

int decrypt;        /* flag to turn on decryption */
char *key;          /* not used--needed to link crypt.c */
int pkzip = 0;      /* set for a pkzip file */
int ext_header = 0; /* set if extended local header */



/* ===========================================================================
 * Unzip in to out.  This routine works on both gzip and pkzip files.
 *
 * IN assertions: the buffer inbuf contains already the beginning of
 *   the compressed data, from offsets inptr to insize-1 included.
 *   The magic header has already been checked. The output buffer is cleared.
 */
int unzip()
{
    ulg orig_crc = 0;       /* original crc */
    ulg orig_len = 0;       /* original uncompressed length */
    int n;
    uch buf[EXTHDR];        /* extended local header */
    int ret = OK;
    
   
    updcrc(NULL, 0);           /* initialize crc */

    
    /* Decompress */
    if (method == DEFLATED)  
    {    
        int res = inflate();

        if (res == 3) 
        {
            printf("out of memory3\n");
            ret = -1;
        } 
        else if (res != 0) 
        {
            printf("invalid compressed data--format violated\n");
            ret = -1;
        }
        
    }
    else 
    {
        printf("internal error, invalid method");
        ret = -1;
    }

    /* Get the crc and original length */
    if (!pkzip) 
    {
        /* crc32  (see algorithm.doc)
         * uncompressed input size modulo 2^32
         */
        for (n = 0; n < 8; n++) 
        {
            buf[n] = (uch)get_byte(); /* may cause an error if EOF */
        }
        orig_crc = LG(buf);
        orig_len = LG(buf+4);

    }
    
#if 0
    else if (ext_header) 
    {   /* If extended header, check it */
        /* signature - 4bytes: 0x50 0x4b 0x07 0x08
         * CRC-32 value
         * compressed size 4-bytes
         * uncompressed size 4-bytes
         */
        for (n = 0; n < EXTHDR; n++) 
        {
            buf[n] = (uch)get_byte(); /* may cause an error if EOF */
        }
        orig_crc = LG(buf+4);
        orig_len = LG(buf+12);
    }
#endif

    /* Validate decompression */
    if (orig_crc != updcrc(outbuf, 0)) 
    {
        printf("invalid compressed data--crc error\n");
        ret = -1;
    }
    if (orig_len != (ulg)bytes_out) 
    {
        printf("invalid compressed data--length error\n");
        ret = -1;
    }


    ext_header = pkzip = 0; /* for next file */
    return ret;
}
