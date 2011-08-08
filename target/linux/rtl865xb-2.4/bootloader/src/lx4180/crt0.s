#/*
# * Copyright c                Realtek Semiconductor Corporation, 2002
# * All rights reserved.                                                    
# * 
# * $Header: /home/cvsroot/uClinux-dist/loader_srcroot/src/lx4180/crt0.s,v 1.8 2004/08/26 13:20:00 yjlou Exp $
# *
# * $Author: yjlou $
# *
# * Abstract:
# *
# *   Lx4180 startup code.
# *
# * $Log: crt0.s,v $
# * Revision 1.8  2004/08/26 13:20:00  yjlou
# * *: Loader upgrades to "00.00.12".
# * +: support "Loader Segment Descriptors Table"
# * -: remove romcopystart/pause/resume
# *
# * Revision 1.7  2004/04/09 09:24:48  yjlou
# * -: DO NOT detect SDRAM when boots from RAM.
# *
# * Revision 1.6  2004/04/01 15:52:49  yjlou
# * +: support detecting SDRAM size.
# * *: MCR will be touched. Therefore, there is still a bug in 51B when detecting multiple flash chips.
# *
# * Revision 1.5  2004/03/31 02:44:18  yjlou
# * *: Flash timing and SDRAM timing have been tuned for better performance.
# * *: configure SDRxCkDly code has move from crt0.s to initmem.s
# *
# * Revision 1.4  2004/03/31 01:49:20  yjlou
# * *: all text files are converted to UNIX format.
# *
# * Revision 1.3  2004/03/30 11:29:19  yjlou
# * *: set SDRxCkDly to 4.78ns in the boot time (to support 166MHz SDRAM)
# *
# * Revision 1.2  2004/03/26 09:31:11  yjlou
# * +: pass isBootFromROM to csp_main()
# *
# * Revision 1.1  2004/03/16 06:36:13  yjlou
# * *** empty log message ***
# *
# * Revision 1.2  2004/03/09 00:42:47  danwu
# * shrink loader image size under 0xc000 and only flash block 0 & 3 occupied
# * now to copy text and data must bypass bdinfo and ccfg blocks
# *
# * Revision 1.1.1.1  2003/09/25 08:16:55  tony
# *  initial loader tree 
# *
# * Revision 1.3  2003/08/29 08:56:42  danwu
# * move rodata section which is inflated to get into bdinfo space
# *
# * Revision 1.2  2003/05/30 10:37:12  danwu
# * support dram 32-bit or 16-bit mode auto-selection
# *
# * Revision 1.1.1.1  2003/05/07 08:16:06  danwu
# * no message
# *
# */



	.globl	_start
	.extern csp_main
	
#/*-------------------------------------------------------------------
#**
#** _start: Performs processor initialization, and starts user's
#**	 application.
#**
#**-------------------------------------------------------------------
#*/
	.section ".start", "ax"
#	.text
	.ent	_start
_start:
	#--- Adjust the current SR to kernel/no interrupts
	.set	noreorder
	mfc0	$9,$12							# load status register
	nop
	srl		$9,$9,2							# clear KUc and IEc
	sll		$9,$9,2
	mtc0	$9,$12							# set status register
	.set	reorder
	
	mfc0	$9,$13
	mtc0	$0,$13							# clear cause register

	#--- flush the caches
	jal		_rom_flush_cache
	
	#--- flush the write buffer
	jal		_rom_wbflush
	
	#--- bypass copying if in ram
	jal		1f
1:
	la		$8,0x10000000	
	and		$8,$31,$8
	move	$15, $8							# save isBootFromROM ?
	beq		$8,$0,_bypass_copy				# decide if ram based on BIT28
	
	#--- init memory
	jal		_initmem
	
	#--- detect SDRAM size and configure MCR
	# jal		_detectSDRAM

	#--- copy code to ram (according to ldrSegDesTable)
	#
	#	$8	start address in RAM
	#	$9	end address in RAM
	#	$10	start address in segment
	#	$11 end address in segment
	#	$12	temp register
	#	$13	pointer to ldrSegDesTable
	.set	noreorder
	
	la		$13,ldrSegDesTable
	la		$8,__ghsbegin_copystart
	la		$9,__ghsend_copyend + 4
1:
	addiu	$13,8							# skip magic number
	lw		$10,0($13)						# load start address of this segment
	lw		$11,4($13)						# load end address of this segment
	la		$12,0xbe000000
	beq		$11,$0,3f						# exit if no more segment
	add		$11,$12							# add flashbase
	add		$10,$12							# add flashbase
2:
	beq		$10,$11,1b						# end of this segment, jump to next segment
	lw		$12,0($10)						# load a word
	beq		$8,$9,3f						# exit if reach end of copy
	addiu	$10,4							# pROM++
	sw		$12,0($8)						# store a word
	addiu	$8,4							# pRAM++
	j		2b
	nop
3:
	.set	reorder

_bypass_copy:

	#--- copy handler to exception vector
	la		$8,0x80000080					# destination start address
	la		$9,__ghsbegin_vectors + 0x80	# source start address
	la		$10,0x80000180					# destination end address
1:
	lw		$11,0($9)
	addiu	$9,4
	sw		$11,0($8)
	addiu	$8,4
	bne		$8,$10,1b
	
	#--- clear bss section
	la		$8,__ghsbegin_bss				# destination start address
	la		$9,__ghsend_bss					# destination end address
	addiu   $9,3
	srl     $9,$9,2
	sll     $9,$9,2
1:
	sw		$0,0($8)
	addiu	$8,4
	bne		$8,$9,1b
	
	#--- init stack
	la		$29,__ghsend_stack
	move	$30,$29
	
	#--- init sda base register, which should point 32 Kbytes past the start of the 
	#--- Small Data Area to provide a full 64 Kbytes of addressing
#	la		$28,__ghsbegin_sdabase
#	addiu	$28,0x4000
#	addiu	$28,0x4000
	la		$28,_gp
	
	#--- start application in C
	la		$4,__ghsbegin_heap					# prepare argument
	move	$5, $15								# restore isBootFromROM ?
	la		$31,csp_main
	j		$31								# first routine in C
	
	.end _start
	
	

#/*-------------------------------------------------------------------
#**
#* _rom_flush_cache
#*
#**-------------------------------------------------------------------
#*/
	## note that _rom_flush_cache is called before the code moving from
	## ROM to RAM, so it should be resident in .start section rather
	## than .text section and it can not share the same code of _flush_cache
	## -xavier
	.ent	_rom_flush_cache
_rom_flush_cache:

	#--- transition to kseg1 from undetermined kernel segment
	la	$9,1f
	or	$9,0xa0000000
	jr	$9

	.set	noreorder

	#--- invalidate the icache and dcache with a 0->1 transition
1:	mtc0	$0, $20	# CCTL
	nop
	nop
	li		$8,0x00000003
	mtc0	$8, $20
	nop
	nop
	
	#--- initialize and start COP3
	mfc0	$8,$12
	nop
	nop
	or		$8,0x80000000
	mtc0	$8,$12
	nop
	nop
	
	#--- load iram base and top
	la		$8,__ghsbegin_iram
	la		$9,0x0ffffc00
	and		$8,$8,$9
	mtc3	$8,$0								# IW bas
	nop
	nop
	addiu	$8,$8,0x1fff
	mtc3	$8,$1								# IW top
	nop
	nop
	
	#--- load dram base and top
	la		$8,0x90000000
	la		$9,0xfffffc00
	and		$8,$8,$9
	mtc3	$8,$4								# DW bas
	nop
	nop
	la		$8,0x1000 - 1
	mtc3	$8,$5								# DW top
	nop
	nop
	
	#--- enable icache and dcache
	mtc0	$0, $20	# CCTL
	nop
	nop

	.set	reorder
	j		$31

	.end	_rom_flush_cache



#/*-------------------------------------------------------------------
#**
#* _rom_wbflush - flush the write buffer
#*
#* This routine flushes the write buffers, making certain all
#* subsequent memory writes have occurred.  It is used during critical periods
#* only, e.g., after memory-mapped I/O register access.
#*
#**-------------------------------------------------------------------
#*/
	.ent	_rom_wbflush
_rom_wbflush:
	li		$8, 0xa0000000				#/* load uncached address	*/
	lw		$8, 0($8)					#/* read in order to flush 	*/
	j		$31							#/* return to caller		*/
	.end	_rom_wbflush



#/*-------------------------------------------------------------------
#**
#** gen_exception_rom: Exceptions come while interrupt not initialized.
#*
#**-------------------------------------------------------------------
#*/
	.globl	_gen_exception_rom
	.ent	_gen_exception_rom
_gen_exception_rom:
	.set	noreorder
	
	#--- debug information
#	mfc0	$26,$12								# load status into k0
#	nop
#	mfc0	$27,$8								# load BADVADDR into k1
#	nop
	mfc0	$26,$13								# load cause into k0
	nop
	mfc0	$27,$14								# load EPC into k1
	nop
	#--- ininite loop as default
	#--- remember to restore registers if handlers are added
	li    $8,0
	li    $9,0x3fffff
	la    $10,0x40000
	la    $11,0x4ffff
1:
  bne   $8,$9,2f
  addiu $8,$8,1
  move  $8,$0
  move  $12,$10
  move  $10,$11
  move  $11,$12
  sw    $10,0xbc805000
2:
	b		  1b
	nop
	
	.set reorder
	.end _gen_exception_rom
	
