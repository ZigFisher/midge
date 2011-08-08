#/*
# * Copyright c                Realtek Semiconductor Corporation, 2002
# * All rights reserved.                                                    
# * 
# * $Header: /home/cvsroot/uClinux-dist/loader_srcroot/src/lx4180/lx4180.s,v 1.4 2004/05/12 06:32:19 yjlou Exp $
# *
# * $Author: yjlou $
# *
# * Abstract:
# *
# *   Lx4180 specific assembly codes.
# *
# * $Log: lx4180.s,v $
# * Revision 1.4  2004/05/12 06:32:19  yjlou
# * -: remove lx4180_ReadCause()
# *
# * Revision 1.3  2004/05/12 05:15:05  tony
# * support PPTP/L2TP in RTL865XB.
# *
# * Revision 1.2  2004/03/31 01:49:20  yjlou
# * *: all text files are converted to UNIX format.
# *
# * Revision 1.1  2004/03/16 06:36:13  yjlou
# * *** empty log message ***
# *
# * Revision 1.2  2004/03/09 00:43:35  danwu
# * remove unused code to shrink loader image size under 0xc000 and only flash block 0 & 3 occupied
# *
# * Revision 1.1.1.1  2003/09/25 08:16:55  tony
# *  initial loader tree 
# *
# * Revision 1.1.1.1  2003/05/07 08:16:06  danwu
# * no message
# *
# */

# current interrupt priority level
	.extern	_currIlev
	
	.text
#/*-------------------------------------------------------------------
#**
#** uint32 lx4180_ReadStatus(void)
#**
#**-------------------------------------------------------------------
#*/
	.globl	lx4180_ReadStatus
	.ent	lx4180_ReadStatus
lx4180_ReadStatus:
	mfc0	$2,$12
	jr		$31
	.end lx4180_ReadStatus




#/*-------------------------------------------------------------------
#**
#** uint32 lx4180_WriteStatus(uint32)
#**
#**-------------------------------------------------------------------
#*/
	.globl	lx4180_WriteStatus
	.ent	lx4180_WriteStatus
lx4180_WriteStatus:
	.globl	writeStatus
#	.aent	writeStatus
writeStatus:
	mfc0	$2,$12
	mtc0	$4,$12
	jr		$31	
	.end lx4180_WriteStatus



#/*-------------------------------------------------------------------
#**
#** uint32 lx4180_ReadCause(void)
#**
#**-------------------------------------------------------------------
#*/
	.globl	lx4180_ReadCause
	.ent	lx4180_ReadCause
lx4180_ReadCause:
	mfc0	$2,$13
	jr		$31	
	.end lx4180_ReadCause



#/*-------------------------------------------------------------------
#**
#** void lx4180_WriteCause(uint32)
#**
#**-------------------------------------------------------------------
#*/
	.globl	lx4180_WriteCause
	.ent	lx4180_WriteCause
lx4180_WriteCause:
	mtc0	$4,$13
	jr		$31	
	.end lx4180_WriteCause



#/*-------------------------------------------------------------------
#**
#** uint32 lx4180_ReadEPC(void)
#**
#**-------------------------------------------------------------------
#*/
	.globl	lx4180_ReadEPC
	.ent	lx4180_ReadEPC
lx4180_ReadEPC:
	mfc0	$2,$14
	jr		$31	
	.end lx4180_ReadEPC



#/*-------------------------------------------------------------------
#**
#** void lx4180_WriteCCTL(uint32)
#**
#**-------------------------------------------------------------------
#*/
	.globl	lx4180_WriteCCTL
	.ent	lx4180_WriteCCTL
lx4180_WriteCCTL:
	mtc0	$4,$20
	jr		$31	
	.end lx4180_WriteCCTL



#/*-------------------------------------------------------------------
#**
#** int32 setIlev(int32 new_ilev)
#**
#**		Setup interrupt level such that only the interrupts of higher 
#**		or equal priority level are permitted. The previous value of 
#**		interrupt level is returned.
#**
#**-------------------------------------------------------------------
#*/
	.globl	setIlev
	.ent	setIlev
setIlev:
	b		TCT_Control_Interrupts
	nop
	.end setIlev

# alternative name to support nucleus
	.globl	TCT_Control_Interrupts
	.ent	TCT_Control_Interrupts
TCT_Control_Interrupts:
	
	.set noreorder
	# mask all interrupts
	mfc0	$8,$12
	nop
	la		$9,0xffff00ff
	and		$10,$8,$9
	mtc0	$10,$12
	nop
	nop

	lw		$2,_currIlev							# pickup the previous value

	# calculate new status register value
	la		$8,0xff00
	sll		$8,$8,$4
	andi	$8,$8,0xff00
	or		$8,$8,$10
	
	sw		$4,_currIlev							# change to new value
	
	# write into status register
	mtc0	$8,$12
	nop
	nop
	jr		$31	
	nop
	.set reorder
	
	.end TCT_Control_Interrupts
	
	
	
#/*-------------------------------------------------------------------
#**
#** int32 getIlev(void)
#**
#**		Get the current interrupt level.
#**
#**-------------------------------------------------------------------
#*/
	.globl	getIlev
	.ent	getIlev
getIlev:
	lw		$2,_currIlev							# pickup the previous value
	nop
	nop
	jr		$31	
	nop
	.end getIlev
    
    

#/*-------------------------------------------------------------------
#*
#** void _saveContext(void)
#**
#**-------------------------------------------------------------------
#*/
	.text
	.globl	_saveContext
	.ent	_saveContext
_saveContext:
	.set	noreorder
	.set	noat
	
	addiu	$26,$29,-128						# allocate stack
												# two more words for gp,sp to support gdb
	sw		$1,4($26)							# backup at
	.set	at
	sw		$2,8($26)							# backup v0,v1
	sw		$3,12($26)
	sw		$4,16($26)							# backup a0~a3
	sw		$5,20($26)
	sw		$6,24($26)
	sw		$7,28($26)
	sw		$8,32($26)							# backup t0~t7
	sw		$9,36($26)
	sw		$10,40($26)
	sw		$11,44($26)
	sw		$12,48($26)
	sw		$13,52($26)
	sw		$14,56($26)
	sw		$15,60($26)
	sw		$16,64($26)							# backup s0~s7 for stub read
	sw		$17,68($26)
	sw		$18,72($26)
	sw		$19,76($26)
	sw		$20,80($26)
	sw		$21,84($26)
	sw		$22,88($26)
	sw		$23,92($26)
	sw		$24,96($26)							# backup t8,t9
	sw		$25,100($26)
	#skip k0,k1
	#skip gp
	#skip sp
	sw		$30,104($26)						# backup s8
	sw		$27,108($26)						# backup ra which is moved to k1 in vector
	
	mfc0	$8,$12								# load status
	nop
	nop
	mfc0	$9,$14								# load EPC
	nop
	nop
	sw		$8,112($26)							# backup status
	sw		$9,116($26)							# backup EPC
	
	# save gp & sp to support gdb
	sw		$28,120($26)
	sw		$29,124($26)
	# setup context base for gdb to read registers
	sw		$26,pExceptionContext
	
	j		$31									# return
	move	$29,$26
	
	.set	reorder
	.end _saveContext
	
	

#/*-------------------------------------------------------------------
#**
#** void _restoreContext(void)
#**
#**-------------------------------------------------------------------
#*/
	.text
	.globl	_restoreContext
	.ent	_restoreContext
_restoreContext:
	.set	noreorder
	.set	noat
	
	move	$26,$29
	lw		$1,4($26)							# restore at
	lw		$2,8($26)							# restore v0,v1
	lw		$3,12($26)
	lw		$4,16($26)							# restore a0~a3
	lw		$5,20($26)
	lw		$6,24($26)
	lw		$7,28($26)
	lw		$8,32($26)							# restore t0~t7
	lw		$9,36($26)
	lw		$10,40($26)
	lw		$11,44($26)
	lw		$12,48($26)
	lw		$13,52($26)
	lw		$14,56($26)
	lw		$15,60($26)
	lw		$16,64($26)							# restore s0~s7 for stub write
	lw		$17,68($26)
	lw		$18,72($26)
	lw		$19,76($26)
	lw		$20,80($26)
	lw		$21,84($26)
	lw		$22,88($26)
	lw		$23,92($26)
	lw		$24,96($26)							# restore t8,t9
	lw		$25,100($26)
	# skip k0,k1
	# skip gp
	# skip sp
	lw		$30,104($26)						# restore s8
	
	lw		$27,112($26)						# restore status
	# restore gp & sp to support gdb
	lw		$28,120($26)
	lw		$29,124($26)
	mtc0	$27,$12								# write status
	nop
	nop
	
	lw		$27,108($26)						# restore ra
	
	j		$31
	lw		$26,116($26)						# fetch saved EPC
	
	.set	at
	.set	reorder
	.end _restoreContext
