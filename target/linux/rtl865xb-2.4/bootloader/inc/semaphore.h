/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home/cvsroot/uClinux-dist/loader_srcroot/inc/semaphore.h,v 1.2 2004/03/31 01:49:20 yjlou Exp $
*
* Abstract: POSIX semaphore definitions.
*
* $Author: yjlou $
*
* $Log: semaphore.h,v $
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


#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H

typedef uint32 sem_t;

/* Semaphore service */
int32 sem_init(sem_t *sem, int32 flag, uint32 value);
int32 sem_wait(sem_t *sem);
int32 sem_post(sem_t *sem);
int32 sem_destroy(sem_t *sem);
int32 sem_trywait(sem_t *sem);
int32 sem_getvalue(sem_t *sem, int32 *sval);

//sem_t *sem_open(const int8 *name, int32 oflag, ... );
//int32 sem_close(sem_t *sem);
//int32 sem_unlink(const int8 *name);
// ------------------------------------

#endif /* _SEMAPHORE_H */
