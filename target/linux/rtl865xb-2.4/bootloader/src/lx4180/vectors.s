#/*
# * Copyright c                Realtek Semiconductor Corporation, 2002
# * All rights reserved.                                                    
# * 
# * $Header: /home/cvsroot/uClinux-dist/loader_srcroot/src/lx4180/vectors.s,v 1.4 2004/11/10 13:02:22 yjlou Exp $
# *
# * $Author: yjlou $
# *
# * Abstract:
# *
# *   Lx4180 vector assembly codes.
# *
# * $Log: vectors.s,v $
# * Revision 1.4  2004/11/10 13:02:22  yjlou
# * *: version migrates to "00.00.15".
# *    +: Integration with Linux menuconfig.
# *    +: add c_data.c
# *    *: change the structure of vectors.s for c_data.c
# *    *: SDRAM map is dynamicallly generated according to menuconfig.
# *
# * Revision 1.3  2004/08/26 13:20:00  yjlou
# * *: Loader upgrades to "00.00.12".
# * +: support "Loader Segment Descriptors Table"
# * -: remove romcopystart/pause/resume
# *
# * Revision 1.2  2004/03/31 01:49:20  yjlou
# * *: all text files are converted to UNIX format.
# *
# * Revision 1.1  2004/03/16 06:36:13  yjlou
# * *** empty log message ***
# *
# * Revision 1.1.1.1  2003/09/25 08:16:55  tony
# *  initial loader tree 
# *
# * Revision 1.1.1.1  2003/05/07 08:16:06  danwu
# * no message
# *
# */

	.globl	_rstvector
	.globl	_ram_int_vector
	.globl	_rom_int_vector
	.extern	_start
	.extern _saveContext
	.extern genexcpt_handler
	.extern _restoreContext

#/*-------------------------------------------------------------------
#**
#**  For the ROM version, the vector section is linked at BFC00000.
#**
#**  BOOTSTRAP EXCEPTION HANDLERS
#**
#**  BFC00000	RESET
#**  BFC00180    Other Exceptions
#**
#**
#**  For the RAM version, the vector section is linked at 0 in kseg0/1.
#**  Note that the entry point in this case will be offset 0x0.
#**
#**  RAM EXCEPTION HANDLERS
#**
#**  80000080    General Exceptions
#**
#**-------------------------------------------------------------------
#*/
	.section ".rstvector", "ax"
#	.text
	.ent	_rstvector
_rstvector:
	j	_start
	nop
	j	_start
	nop
	j	_start
	nop
	j	_start
	nop
	j	_start
	nop
	j	_start
	nop
	j	_start
	nop
	j	_start
	nop
	j	_start
	nop
	j	_start
	nop
	.end _rstvector
	
	#--- General exception handler for RAM, offset 80
	#.align  7	#force next address to multiple of 0x80
	.set noreorder
	.set noat
	.section ".ram_int_vector", "ax"
	.ent	_ram_int_vector
_ram_int_vector:
	
	move	$27,$31								# backup ra
	la		$26,_saveContext
	jal		$26
	nop
	la		$26,genexcpt_handler
	jal		$26
	nop
	la		$26,_restoreContext
	jal		$26
	nop
	move	$31,$27								# restore ra
	j		$26									# return from exception
	rfe
	
	.set at
	.set reorder
	
	.end	_ram_int_vector
	
	#.align  8	#force next address to multiple of 0x100
	#nop
	

	#--- General exception handler for ROM, offset 180
	.align  7	#force next address to multiple of 0x80
	.section ".rom_int_vector", "ax"
	.ent	_rom_int_vector
_rom_int_vector:
	la	$26, _gen_exception_rom
	jr	$26
	.end	_rom_int_vector
	

