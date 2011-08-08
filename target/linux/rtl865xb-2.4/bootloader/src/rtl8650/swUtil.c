/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home/cvsroot/uClinux-dist/loader_srcroot/src/rtl8650/swUtil.c,v 1.3 2004/07/14 02:16:09 yjlou Exp $
*
* Abstract: Utilities for switch core drivers.
*
* $Author: yjlou $
*
* $Log: swUtil.c,v $
* Revision 1.3  2004/07/14 02:16:09  yjlou
* +: add '#ifdef FAT_CODE' to remove un-used functions
*
* Revision 1.2  2004/03/31 01:49:20  yjlou
* *: all text files are converted to UNIX format.
*
* Revision 1.1  2004/03/16 06:36:13  yjlou
* *** empty log message ***
*
* Revision 1.1.1.1  2003/09/25 08:16:55  tony
*  initial loader tree 
*
* Revision 1.2  2003/06/19 05:28:51  danwu
* replace s2Mac() with strToMac()
*
* Revision 1.1.1.1  2003/05/07 08:16:06  danwu
* no message
*
* ---------------------------------------------------------------
*/

#include <rtl_types.h>



/*
 * Convert an ASCII string to a
 * binary representation of mac address
*/
int32 strToMac(uint8 *pMac, int8 *pStr)
{
    int8 *ptr;
    uint32 i,k;
    
    assert(pMac);
    assert(pStr);
    
    bzero(pMac,6);
    ptr = pStr;
    for(k=0;*ptr;ptr++)
    {
        if( (*ptr==':') || (*ptr=='-') )
            k++;
        else if( ('0'<=*ptr) && (*ptr<='9') )
            pMac[k]=(pMac[k]<<4)+(*ptr-'0');
        else if( ('a'<=*ptr) && (*ptr<='f') )
            pMac[k]=(pMac[k]<<4)+(*ptr-'a'+10);
        else if( ('A'<=*ptr) && (*ptr<='F') )
            pMac[k]=(pMac[k]<<4)+(*ptr-'A'+10);
        else
            break;
    }
    if(k!=5)
        return -1;
    
    return 0;
}

#ifdef FAT_CODE
int32 s2IP(ipaddr_t *ip_P, int8 * str_P)
{
    uint32  val;
    uint32  count = 1;
    
    ASSERT_CSP( ip_P );
    ASSERT_CSP( str_P );
    
    *ip_P = 0;
    
    for (val = 0; *str_P; str_P++)
    {
        if ( '0' <= *str_P && *str_P <= '9' )
        {
            val *= 10;
            val += *str_P - '0';
        }
        else if ( *str_P == '.' )
        {
            *ip_P |= (val & 0xff) << (32 - 8 * count);
            
            val = 0;
            count++;
        }
        else
            break;
    }
    *ip_P |= (val & 0xff);
    
    return 0;
}

uint32 s2IPMask(int8 * str_P)
{
    uint32  val;
    uint32  ipMask;
    uint32  count = 1;
    
    ASSERT_CSP( str_P );
    
    ipMask = 0;
    
    for (val = 0; ; str_P++)
    {
        if ( '0' <= *str_P && *str_P <= '9' )
        {
            val *= 10;
            val += *str_P - '0';
        }
        else if (*str_P == '.')
        {
            if (val == 255)
                ipMask += 8;
            else
                break;
            
            val = 0;
            count++;
        }
        else
            break;
    }
    if (val == 255)
        ipMask += 8;
    else
    {
        if (val < 128);
        else if ((val >= 128) && (val < 192))
            ipMask += 1;
        else if ((val >= 192) && (val < 224))
            ipMask += 2;
        else if ((val >= 224) && (val < 240))
            ipMask += 3;
        else if ((val >= 240) && (val < 248))
            ipMask += 4;
        else if ((val >= 248) && (val < 252))
            ipMask += 5;
        else if ((val >= 252) && (val < 254))
            ipMask += 6;
        else if (val == 254)
            ipMask += 7;
    }
    
    return ipMask;
}

int32 s2i(int8 * str_P)
{
    uint32  val;
    
    if ( (str_P[0] == '0') && (str_P[1] == 'x') )
    {
        str_P += 2;
        for (val = 0; *str_P; str_P++)
        {
            val *= 16;
            if ( '0' <= *str_P && *str_P <= '9' )
                val += *str_P - '0';
            else if ( 'a' <= *str_P && *str_P <= 'f' )
                val += *str_P - 'a' + 10;
            else if ( 'A' <= *str_P && *str_P <= 'F' )
                val += *str_P - 'A' + 10;
            else
                break;
        }
    }
    else
    {
        for (val = 0; *str_P; str_P++)
        {
            val *= 10;
            if ( '0' <= *str_P && *str_P <= '9' )
                val += *str_P - '0';
            else
                break;
        }
    }
    
    return val;
}
#endif//FAT_CODE

