/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home/cvsroot/uClinux-dist/loader_srcroot/inc/compiler/gnu/rtl_depend.h,v 1.2 2004/03/31 01:49:20 yjlou Exp $
*
* Abstract: Header file for gnu platform.
*
* $Author: yjlou $
*
* $Log: rtl_depend.h,v $
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

#ifndef _RTL_DEPEND_H_
#define _RTL_DEPEND_H_


/*
 * many standard C library API references "size_t"
 * cygwin's stdio.h and ghs's stdio.h will test
 * _SIZE_T symbol before typedef ...
 */
#ifndef _SIZE_T
#define _SIZE_T
	typedef unsigned int size_t;
#endif /* _SIZE_T */

#define RTL_EXTERN_INLINE
#define RTL_STATIC_INLINE   static __inline__

#define malloc(x) osk_malloc(x)
#define free(x) osk_free(x)

//Our own assert macro, when condition not met, forces cygwin to create a stackdump file.
#undef assert
#define assert(x)\
if (!(x)) { \
        int *p=NULL;\
        printf("\nAssertion fail at File %s, In function %s, Line number %d:\nExpression '%s'", __FILE__, __FUNCTION__, __LINE__, #x);\
	*p = 123;\
	while(1){};\
}\


#endif   /* _RTL_DEPEND_H_ */
