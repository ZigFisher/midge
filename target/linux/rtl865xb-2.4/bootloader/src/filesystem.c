/*
 * Copyright c                Realtek Semiconductor Corporation, 2002
 * All rights reserved.                                                    
 * 
 * $Header: /home/cvsroot/uClinux-dist/loader_srcroot/src/filesystem.c,v 1.12 2005/08/10 05:51:49 ghhuang Exp $
 *
 * $Author: ghhuang $
 *
 * Abstract:
 *
 * filesys.c
 *
 *   File system access source codes.
 *
 * $Log: filesystem.c,v $
 * Revision 1.12  2005/08/10 05:51:49  ghhuang
 * *: Change GZIP/LZMA decompression message
 *
 * Revision 1.11  2005/07/15 11:33:05  ghhuang
 * +: Loader Revision 00.00.20 with LZMA
 *
 * Revision 1.10  2005/06/17 07:37:43  ghhuang
 * +: LZMA support
 *
 * Revision 1.9  2005/03/08 02:57:10  yjlou
 * *: fixed the max size protection code of fileSys_updateROMImage()
 *
 * Revision 1.8  2005/01/19 11:46:33  yjlou
 * +: support _SUPPORT_UPDATE_ROM_
 * *: fixed th bug of program bdinfo: we should use FLASH_MAP_BOARD_INFO_ADDR, instead of flash_chip_info[0].BoardInfo.
 *
 * Revision 1.7  2005/01/12 13:47:54  yjlou
 * -: don't show "Start image at address ...".
 *
 * Revision 1.6  2004/08/26 13:20:00  yjlou
 * *: Loader upgrades to "00.00.12".
 * +: support "Loader Segment Descriptors Table"
 * -: remove romcopystart/pause/resume
 *
 * Revision 1.5  2004/08/19 02:22:26  rupert
 * +: Support Kernel+Root FS
 *
 * Revision 1.4  2004/08/04 14:55:23  yjlou
 * +: Loader version upgraded to '00.00.09'
 * +: support booting from single 8MB/16MB flash (_SUPPORT_LARGE_FLASH_)
 * -: merge rtl_bdinfo.h into flashdrv.h
 *
 * Revision 1.3  2004/05/13 13:27:01  yjlou
 * +: loader version is migrated to "00.00.07".
 * +: new architecture for INTEL flash (code is NOT verified).
 * *: FLASH_BASE is decided by IS_REV_A()
 * -: remove flash_map.h (content moved to flashdrv.h)
 * -: remove un-necessary calling setIlev()
 *
 * Revision 1.2  2004/03/26 10:10:01  yjlou
 * *: change bdinfo, ccfg, and runtime offset from const to variable.
 *    Therefore, this version of loader can autosense the flash type and address map.
 *    Now, we support AMD, fujitsu, ST(for Buffalo) flashes.
 * +: support ICE download mode
 * *: fixed the bug: crash after setting default board info.
 *
 * Revision 1.1  2004/03/16 06:36:13  yjlou
 * *** empty log message ***
 *
 * Revision 1.2  2004/03/08 13:33:47  danwu
 * shrink loader image size under 0xc000 and only flash block 0 & 3 occupied
 *
 * Revision 1.1.1.1  2003/09/25 08:16:55  tony
 *  initial loader tree 
 *
 * Revision 1.2  2003/06/19 05:30:18  danwu
 * verify checksum before loading runtime image
 *
 * Revision 1.1.1.1  2003/05/07 08:16:06  danwu
 * no message
 *
 */

#include <rtl_types.h>
#include <board.h>
#include <flashdrv.h>
#include <rtl_image.h>
#include <rtl_flashdrv.h>
#include "./gzip/gzip.h"
#include "./lzma/lzma.h"

extern bdinfo_t	bdinfo;


/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
RTL_STATIC_INLINE int fileSys_checkImage(void *, uint32);

#ifdef _SUPPORT_UPDATE_ROM_
int fileSys_updateROMImage(void * baseAddr, uint32 size)
{
    fileImageHeader_t * imghdr;
    uint32              imask;
    uint32 nSwapped;
    volatile uint16* pSwap;
    
    ASSERT_CSP((uint32 *)baseAddr);

    /*
     *  Do byte-swap first.
     */
    for( nSwapped = 0, pSwap = baseAddr;
         nSwapped < ((size+1)>>1);
         nSwapped++, pSwap++ )
    {
    	uint16 tmp;
    	tmp = ((*pSwap)>>8) | (((*pSwap)&0xff)<<8);
    	*pSwap = tmp;
    }

    /* Check length */
    if ( size > flashdrv_getDevSize() )
    {
        debug_printf("\nImage too fat: image=%d bytes,  flash size=%d bytes.\n", size, flashdrv_getDevSize() );
        return -1;
    }
    
    printf("\nWriting FlashROM %08x~%08x ... ", 
            flashdrv_getBlockBase(0),
            flashdrv_getBlockBase(0) + size - 1);
    tick_Delay100ms(5);
    
    /* Disable all interrupts */
    //imask = setIlev(MAX_ILEV);
#if 1
	if ( flashdrv_updateImg(
	            baseAddr, (void *) ( flashdrv_getBlockBase(0) ), size) )
    {
        //setIlev(imask);
        printf("FAIL !\n\n");
        return -1;
    }
#else
	/* dump for debug */
	{
		uint8 *pDump;
		uint32 i;
		for( i = 0, pDump = baseAddr;
		     i < size && i < 0x200;
		     i++, pDump++ )
		{
			if ( ( i & 0xf ) == 0x0 ) printf("\n%05x: ", i );
			printf( "%02x:", *pDump );
		}
	}
#endif

    //setIlev(imask);
    printf("SUCCESS !");
    return 0;
}
#endif/*_SUPPORT_UPDATE_ROM_*/


int fileSys_updateRuntimeImage(void * baseAddr, uint32 size)
{
    fileImageHeader_t * imghdr;
    uint32              imask;
    
    ASSERT_CSP((uint32 *)baseAddr);
    ASSERT_CSP(size > sizeof(fileImageHeader_t));
    
    imghdr = (fileImageHeader_t *) baseAddr;
    
    /* Check type magic number */
    if ( (imghdr->imageType != RTL_IMAGE_TYPE_RUN) &&
	 (imghdr->imageType != RTL_IMAGE_TYPE_KFS ) )
    {
		//neo@
        debug_printf("\nImage type error! (0x%02X)", imghdr->imageType);
        return -1;
    }

    /* Check length */
    if ( size > bdinfo.rootMaxSize )
    {
        debug_printf("\nImage too fat!");
        return -1;
    }
    
    if ( fileSys_checkImage(baseAddr, size) )
    {
        printf("\nIncorrect image header !");
        return -1;
    }
            
    printf("\nWriting FlashROM %08x~%08x ... ", 
            flashdrv_getBlockBase(0) + bdinfo.rootStartOffset,
            flashdrv_getBlockBase(0) + bdinfo.rootStartOffset + size - 1);
    tick_Delay100ms(5);
    
    /* Disable all interrupts */
    //imask = setIlev(MAX_ILEV);
    
	if ( flashdrv_updateImg(
	            baseAddr, (void *) ( flashdrv_getBlockBase(0) + bdinfo.rootStartOffset ), size) )
    {
        //setIlev(imask);
        printf("FAIL !\n\n");
        return -1;
    }
    
    //setIlev(imask);
    printf("SUCCESS !");
    return 0;
}


/* segment descriptor of old-falshion loader, for compatibility */
struct LDR_SEG_DESC oldFashion[] =
{
	{ 0x00000000, 0x00004000 },
	{ 0x00008000, 0x00020000 },
	{ 0x00000000, 0x00000000 }
};


int fileSys_updateBootImage(void * baseAddr, uint32 size)
{
    fileImageHeader_t * imghdr;
#if 1
	struct LDR_SEG_DESC *ldrSegDescTable;
	uint8 *srcAddr;
#else/*1*/
    uint32              imask;
	uint32 firstBlockSize = FLASH_ROMCOPYPAUSE - FLASH_ROMCOPYSTART;
#endif/*1*/
    
    ASSERT_CSP(baseAddr);
    ASSERT_CSP(size > sizeof(fileImageHeader_t));
    
    imghdr = (fileImageHeader_t *) baseAddr;
    
    /* Check type magic number */
    if ( imghdr->imageType != RTL_IMAGE_TYPE_BOOT )
    {
		//neo@
        debug_printf("\nImage type error! (0x%02X)", imghdr->imageType);
        return -1;
    }

    if ( fileSys_checkImage(baseAddr, size) )
    {
        printf("\nIncorrect image header !");
        return -1;
    }
            
    printf("\nWriting FlashROM ... ");
    tick_Delay100ms(5);
    
    /* Disable all interrupts */
    //imask = setIlev(MAX_ILEV);
    
#if 1
	/**** Update flash according Loader Segment Descriptor Table ****/
	ldrSegDescTable = (struct LDR_SEG_DESC *)((uint32)baseAddr+sizeof(fileImageHeader_t)+
	                                           LDR_SEG_DESC_TABLE_OFFSET);
	if ( ldrSegDescTable->start==LDR_SEG_DESC_MAGIC &&
	     ldrSegDescTable->end==LDR_SEG_DESC_MAGIC )
	{
		/* The image is a new-fashion loader.
		 * convert negtive offset by flash size */
		int32 flashsize ;
		struct LDR_SEG_DESC *tmp;

		flashsize = flashdrv_getDevSize();
		tmp = ++ldrSegDescTable;  /* skip magic number */
		while ( tmp->end )
		{
			if ( tmp->start < 0 )
			{
				tmp->start = flashsize + tmp->start;
			}
			if ( tmp->end < 0 )
			{
				tmp->end = flashsize + tmp->end;
			}
			tmp++;
		}
	}
	else
	{
		/* The image is a old-fashion loader. */
		ldrSegDescTable = oldFashion;
	}

	/* ldrSegDescTable is pointed to segment descriptor table now (magic number is skipped). */
	srcAddr = (uint8*)((uint32) baseAddr + sizeof(fileImageHeader_t));
	while ( ldrSegDescTable->end )
	{
		uint32 blockSize;
		uint8 *dstAddr;

		blockSize = ldrSegDescTable->end - ldrSegDescTable->start;
		dstAddr = (uint8*)(flashdrv_getBlockBase(0) + ldrSegDescTable->start);
		if ( flashdrv_updateImg(
		        (void *) srcAddr,
		        (void *) dstAddr,
		        min(size, blockSize) ) )
		{
			printf("FAIL at %p\n\n", dstAddr);
			return -1; 
		}
		
		srcAddr += blockSize;
		if ( size <= blockSize ) break; /* EOF */
		size -= blockSize;
		ldrSegDescTable++;
	}
	
#else/*1*/

	/*********** 1st segment ************/
    /* Remember to rip the header off */
    if ( flashdrv_updateImg(
	            (void *) ((uint32) baseAddr + sizeof(fileImageHeader_t)), 
	            (void *) flash_chip_info[0].BlockBase,
	            firstBlockSize ) )
    {
        //setIlev(imask);
        printf("FAIL !\n\n");
        return -1;
    }

	if ( size - sizeof(fileImageHeader_t) > firstBlockSize )
	{
		/*********** 2nd segment ************/

		/* preserve next two sections for board info and root directory */
		if ( flashdrv_updateImg( 
		     (void *) ((uint32) baseAddr + sizeof(fileImageHeader_t) + ( FLASH_ROMCOPYPAUSE - FLASH_ROMCOPYSTART ) ),  
		     (void *) flash_chip_info[0].BlockBase + FLASH_ROMCOPYRESUME,  
		     size - sizeof(fileImageHeader_t) - firstBlockSize ) )
		{ 
			//setIlev(imask); 
			printf("FAIL !\n\n"); 
			return -1; 
		} 
	}
	else
	{
		/* Image is too small to write. */
	}
#endif/* 1 */

#ifdef _SUPPORT_LARGE_FLASH_
	/*********** Inject JUMP code ************/
	flashdrv_InjectJumpInstructions();
#endif/*_SUPPORT_LARGE_FLASH_*/
	
    //setIlev(imask);
    printf("SUCCESS !");
    return 0;
}

int fileSys_loadRuntimeImage(void)
{
    fileImageHeader_t * imghdr;
    flash_file          infile, outfile;     /* gunzip in/out file */
    
    imghdr = (fileImageHeader_t *) ( flashdrv_getBlockBase(0) + bdinfo.rootStartOffset );
    
    /* Check type magic number */
    if ( (imghdr->imageType != RTL_IMAGE_TYPE_RUN) &&
	 (imghdr->imageType != RTL_IMAGE_TYPE_KFS) )
    {
        debug_printf("\nRun image might be corrupted.");
        return -2;
    }
    
    if ( fileSys_checkImage((void*) imghdr, imghdr->imageLen + sizeof(fileImageHeader_t)) )
    {
        printf("\nIncorrect image header !");
        return -1;
    }
        
    /* unzip download buffer */
    
    if((bdinfo.ramStartAddress & 0xf0000000)!=0x80000000)  
    {
    	printf("\nCan't start image at address: 0x%x\n",bdinfo.ramStartAddress);
   		return -3;
    }
    printf("\nUnzip image from address: 0x%08x\n", (uint32) imghdr);
    if ( imghdr->imageType == RTL_IMAGE_TYPE_KFS )
		imghdr++;
    infile.buffer = (char *) ((uint32) imghdr + sizeof(fileImageHeader_t));   		
    outfile.buffer = (char *)bdinfo.ramStartAddress;
    infile.size = imghdr->imageLen;

#ifdef CONFIG_RTL865X_LZMA_DECOMPRESSION
    debug_printf("Start LZMA Decompression:");
    if ( lzma_decompress(infile.buffer, outfile.buffer, infile.size) )
    {
        debug_printf(" Failed\n");
        return -2;
    }
#endif

#ifdef CONFIG_RTL865X_GZIP_DECOMPRESSION
    debug_printf("Start GZIP Decompression:");
    if ( gzip_decompress(&infile, &outfile) )
    {
        debug_printf(" Failed\n");
        return -2;
    }
#endif

#ifdef CONFIG_RTL865X_ALL_DECOMPRESSION
    debug_printf("Start GZIP Decompression:");
   if (gzip_decompress(&infile, &outfile))
   {
       debug_printf(" Failed\n");
       debug_printf("Start LZMA Decompression:");
       if ( lzma_decompress(infile.buffer, outfile.buffer, infile.size) )
	{
	       debug_printf(" Failed\n");
	       return -2;
	}
   }
#endif

    //printf("\nStart image at address: 0x%x\n",bdinfo.ramStartAddress);
    //tick_Delay100ms(5);
                
    return 0;
}

RTL_STATIC_INLINE int fileSys_checkImage(void * baseAddr, uint32 size)
{
    fileImageHeader_t * imghdr;
    uint8 *             pdata;
    int               i;
    int               len;
    uint8               chksum;
#ifdef _SUPPORT_LARGE_FLASH_
	uint32 startOffset       = 0; /* start offset to write */
	uint32 endOffset         = 0; /* end offset to write */
	uint32 InstructionOffset = 0; /* offset of boot instruction */
	int32 isOverlappedBootInst = FALSE;
	uint8 *startOver, *endOver;
    
    if ( ((((uint32)baseAddr) & 0xffc00000) == 0xbfc00000 ) ||
         ((((uint32)baseAddr) & 0xff000000) == 0xbe000000 ) )
    {
		startOffset = ((uint32)baseAddr) - flashdrv_getFlashBase();
		endOffset = startOffset + size;
		isOverlappedBootInst = flashdrv_isOver5xBBootInstructions( startOffset, endOffset, &InstructionOffset );
		startOver = (uint8*)(InstructionOffset + flashdrv_getFlashBase());
		endOver = startOver + sizeof(bdinfo.BackupInst);
    }
#endif/*_SUPPORT_LARGE_FLASH_*/
    
    imghdr = (fileImageHeader_t *) baseAddr;
    
    /* Check product magic number */
    if (imghdr->productMagic != RTL_PRODUCT_MAGIC )
    {
        debug_printf("\nImage product magic number error!");
        return -1;
    }
    
    /* Check image header checksum */
    pdata = (uint8 *) imghdr;
    len = sizeof(fileImageHeader_t) - 1;
    chksum = 0;

#ifdef _SUPPORT_LARGE_FLASH_ 
	if ( isOverlappedBootInst )
	{
	    for (i=0; i<len; i++)
	    {
	    	if ( pdata >= startOver &&
			     pdata < endOver )
			{
	    		uint8 *p = (uint8*)bdinfo.BackupInst;
				chksum ^= p[pdata-startOver];
				pdata++;
			}
			else
			{
				chksum ^= *pdata++;
			}
		}
	}
	else
	{
	    for (i=0; i<len; i++)
	    {
			chksum ^= *pdata++;
		}
	}
#else/*_SUPPORT_LARGE_FLASH_*/
    for (i=0; i<len; i++)
        chksum ^= *pdata++;
#endif/*_SUPPORT_LARGE_FLASH_*/	

	/* neo@
    if ( chksum != imghdr->imageHdrCksm )
    {
        debug_printf("\nImage header checksum error!");
        return -1;
    }
    */

    /* Check body checksum */
    pdata = (uint8 *) ((uint32) baseAddr + sizeof(fileImageHeader_t));
    len = (size - sizeof(fileImageHeader_t));
    chksum = 0;

#ifdef _SUPPORT_LARGE_FLASH_
 	if ( isOverlappedBootInst )
	{
	    for (i=0; i<len; i++)
	    {
	    	if ( pdata >= startOver &&
			     pdata < endOver )
			{
	    		uint8 *p = (uint8*)bdinfo.BackupInst;
				chksum ^= p[pdata-startOver];
				pdata++;
			}
			else
			{
				chksum ^= *pdata++;
			}
		}
	}
	else
	{
	    for (i=0; i<len; i++)
	    {
			chksum ^= *pdata++;
		}
	}
#else/*_SUPPORT_LARGE_FLASH_*/
    for (i=0; i<len; i++)
        chksum ^= *pdata++;
#endif/*_SUPPORT_LARGE_FLASH_*/

	/* neo@
    if ( chksum != imghdr->imageBdyCksm )
    {
        debug_printf("\nImage checksum error!");
        return -1;
    }
    */

    return 0;
}

