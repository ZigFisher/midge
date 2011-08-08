#ifndef DEBUG_H
#define DEBUG_H

//#define APP_DEBUG

extern int debug_lev;

#define DOFF -1
#define DERR 0
#define DINFO 5
#define DFULL 10

#ifdef APP_DEBUG
//#	include <cassert>
//#	define ASSERT(x) assert(x)
#   define PDEBUG(lev,fmt,args...) \
		if( lev<=debug_lev ) \
			printf("mxconfig(%s): " fmt " \n",__FUNCTION__, ## args  )


#	define EDEBUG(lev,function) \
		if( lev<=debug_lev ) \
			function

#	define PERROR(fmt,args...) printf("eocd(%s) " fmt " : %s\n", __FUNCTION__, ##args,strerror(errno))

#else
#	define ASSERT(x) 
#	define PDEBUG(lev,fmt,args...)
#	define EDEBUG(lev,function)
#	define PERROR(fmt,args...)

#endif


#endif


