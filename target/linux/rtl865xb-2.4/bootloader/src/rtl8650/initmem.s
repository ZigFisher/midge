#/*
# * Copyright c                Realtek Semiconductor Corporation, 2002
# * All rights reserved.                                                    
# * 
# * $Header: /home/cvsroot/uClinux-dist/loader_srcroot/src/rtl8650/initmem.s,v 1.24 2004/11/17 14:10:24 yjlou Exp $
# *
# * $Author: yjlou $
# *
# * Abstract:
# *
# *   Memory controller initialization code.
# *
# * $Log: initmem.s,v $
# * Revision 1.24  2004/11/17 14:10:24  yjlou
# * -: do not run testSDRAM
# *
# * Revision 1.23  2004/11/15 04:35:28  yjlou
# * -: remove regMcr0_sdram_bus_width (use regMcr0_sdram instead).
# *
# * Revision 1.22  2004/11/10 13:02:22  yjlou
# * *: version migrates to "00.00.15".
# *    +: Integration with Linux menuconfig.
# *    +: add c_data.c
# *    *: change the structure of vectors.s for c_data.c
# *    *: SDRAM map is dynamicallly generated according to menuconfig.
# *
# * Revision 1.21  2004/11/01 08:36:06  yjlou
# * *: we no longer support auto-detect SDRAM 16/32-bit mode.
# *
# * Revision 1.20  2004/08/13 01:32:47  yjlou
# * *: SDRAM size default set to 32MB for demo board.
# *
# * Revision 1.19  2004/07/01 03:19:08  yjlou
# * *: MCR in 50B is wrong!
# *
# * Revision 1.18  2004/06/29 08:07:31  yjlou
# * *: Stable MCR timing for 5xB demo board
# *
# * Revision 1.17  2004/05/26 06:51:49  yjlou
# * *: use IS_865XB() instead of IS_REV_B()
# * *: use IS_865XA() instead of IS_REV_A()
# *
# * Revision 1.16  2004/05/26 04:43:05  yjlou
# * *: 50A and 50B use different MCR setting.
# *
# * Revision 1.15  2004/05/18 09:55:06  yjlou
# * +: detect chip revision, then set different memory timing (in _initmem()).
# *
# * Revision 1.14  2004/05/06 03:57:08  yjlou
# * *: finr tune testSDRAM()
# *    *: fixed the bug of Timer counter mode in 50A
# *    *: fixed the bug of GPIO configuration
# *    *: MCR is set to 0xe2b00000
# *
# * Revision 1.13  2004/05/05 15:17:49  yjlou
# * +: support SDRAM test for BUFFALO_BBR2_4HG. Blink LEDs when SDRAM test failed.
# *    +: testSDRAM()
# *    +: BlinkLED()
# *    +: counter_delay100ms
# *
# * Revision 1.12  2004/04/27 04:56:57  yjlou
# * +: enable testing SDRAM 32-bit mode. (MCR default sets to 32 bit)
# *
# * Revision 1.11  2004/04/14 01:33:02  yjlou
# * *: MCR default set to 16-bit, 16MB DRAM, 2MB ROM, 200MHz (16MB SDRAM for runtime code)
# *
# * Revision 1.10  2004/04/13 12:59:30  yjlou
# * -: DO NOT touch MCR in _initmem()
# *
# * Revision 1.9  2004/04/12 08:39:41  yjlou
# * *: tempary DO NOT detect 32bit mode in boot time
# *
# * Revision 1.7  2004/04/05 01:48:54  yjlou
# * *: SDRAM timing is tuned for ryan's sugguestion.
# *
# * Revision 1.6  2004/04/01 15:52:50  yjlou
# * +: support detecting SDRAM size.
# * *: MCR will be touched. Therefore, there is still a bug in 51B when detecting multiple flash chips.
# *
# * Revision 1.5  2004/03/31 13:51:57  yjlou
# * *: Fixed the bug of multiple flash chips CANNOT be detected: DO NOT touch MCR.
# *
# * Revision 1.4  2004/03/31 02:44:18  yjlou
# * *: Flash timing and SDRAM timing have been tuned for better performance.
# * *: configure SDRxCkDly code has move from crt0.s to initmem.s
# *
# * Revision 1.3  2004/03/31 01:49:20  yjlou
# * *: all text files are converted to UNIX format.
# *
# * Revision 1.2  2004/03/30 11:29:19  yjlou
# * *: set SDRxCkDly to 4.78ns in the boot time (to support 166MHz SDRAM)
# *
# * Revision 1.1  2004/03/16 06:36:13  yjlou
# * *** empty log message ***
# *
# * Revision 1.1.1.1  2003/09/25 08:16:55  tony
# *  initial loader tree 
# *
# * Revision 1.3  2003/08/29 08:57:13  danwu
# * set mcr according to dram size
# *
# * Revision 1.2  2003/05/30 10:37:22  danwu
# * support dram 32-bit or 16-bit mode auto-selection
# *
# * Revision 1.1.1.1  2003/05/07 08:16:06  danwu
# * no message
# *
# */

#/*-------------------------------------------------------------------
#**
#**  !!! WARNING !!! WARNING !!! WARNING !!! WARNING !!! WARNING !!!
#**
#**  Register assignment in initmem.s
#**
#**  $4   -- reserved for function argument
#**  $5   -- reserved for function argument
#**  $8   -- reserved for function argument
#**  $9   -- reserved for function argument
#**  $10  -- reserved for function argument
#**  $11  -- reserved for testSDRAM() to save $31
#**  $22  -- reserved for _detectSDRAM() to save $31
#**  $23  -- reserved for _initmem() to save $31
#**
#**
#**-------------------------------------------------------------------
#*/

		
#/*-------------------------------------------------------------------
#**
#* $4  -- MCR value
#* $5  -- Address of MCR
#*
#* uint32 _tryWriteRAM( $8, $9, $10 )
#*  $8 = RAM boundary address
#*  $9 = ROMSize value in MCR
#*  $10 = dummy address
#*
#* return value($9): 0 -- SUCCESS
#*                   1 -- FAILED
#*
#**-------------------------------------------------------------------
#*/
	.section ".initmem", "ax"
#	.text
	.globl	_tryWriteRAM
	.ent	_tryWriteRAM
_tryWriteRAM:
	li		$11,0xcffff7ff	# Mask for RAM Size fields
	and		$4,$4,$11
	or		$4,$4,$9
	sw		$4,0($5)

	lw		$9,0($8)		# Read data in testing RAM
	not		$9				# NOT the data
	sw		$9,0($8)		# Store the NOTed data back
	lw		$10,0($10)		# load a dummy data
	lw		$10,0($8)		# Read again
	beq		$9,$10,1f
	#--- data is not equal, RAM is not existed.
	li		$9,1	# FAILED
	j		2f
1:
	#--- data is equal, RAM is existed.
	li		$9,0	# SUCCESS
2:
	jr		$31
	.end	_tryWriteRAM
	

#/*-------------------------------------------------------------------
#**
#* void _detectSDRAM(void)
#*
#**-------------------------------------------------------------------
#*/
	.section ".initmem", "ax"
#	.text
	.globl	_detectSDRAM
	.ent	_detectSDRAM
_detectSDRAM:
	move	$22,$31				# save return PC

	# load MCR address and value in $5 and $4
	li		$5,0xBD013000
    lw		$4,0($5)

	#--- test if 64MB SDRAM works ?
	la		$9,0x30000800
	la		$8,0x84000000-4
	la		$10,0x82000000-4
	jal		_tryWriteRAM
	beq		$9,$0,1f

	#--- test if 32MB SDRAM works ?
	la		$9,0x30000000
	la		$8,0x82000000-4
	la		$10,0x81000000-4
	jal		_tryWriteRAM
	beq		$9,$0,1f

	#--- test if 16MB SDRAM works ?
	la		$9,0x20000000
	la		$8,0x81000000-4
	la		$10,0x80800000-4
	jal		_tryWriteRAM
	beq		$9,$0,1f

	#--- test if 8MB SDRAM works ?
	la		$9,0x10000000
	la		$8,0x80800000-4
	la		$10,0x80400000-4
	jal		_tryWriteRAM
	beq		$9,$0,1f

/****
	#--- test if 2MB SDRAM works ?
	la		$9,0x00000300
	la		$8,0x80200000-4
	jal		_tryWriteRAM
*****/

1:

/******************
	#--- test if 16MB DRAM works
	sw		$0,0xa0000000
	la		$8,0x55aa33cc
	sw		$8,0xa0800000
	lw		$9,0xa0000000
	beq		$9,$0,1f
	#--- 16MB DRAM does not work, change to 8MB DRAM
	li		$8,0xcfffffff
	and		$4,$4,$8
	li		$8,0x10000000
	or		$4,$4,$8
	sw		$4,0($5)
1:
*******************/
	move	$31,$22				# restore return PC
	jr		$31
	.end	_detectSDRAM



#/*-------------------------------------------------------------------
#**
#* void counter_delay100ms( $4 = units )
#*
#*  $8,$9,$10 will be used.
#*
#**-------------------------------------------------------------------
#*/
	.section ".initmem", "ax"
#	.text
	.globl	counter_delay100ms
	.ent	counter_delay100ms

counter_delay100ms:

	li		$10,0
2:
	
	# delay 100ms
	li		$8,0xbd012038 # DIVISOR
	li		$9,0x000e0000 # 0x0E
	sw		$9,0($8)
	
	li		$8,0xbd012030 # TCCNR
	li      $9,0x80000000 # TC0EN | TC0MODE_COUNTER
	sw		$9,0($8)

	li		$8,0xbd012020 # TC0DATA
	li      $9,0x0AE63200 # 100ms
	sw		$9,0($8)

	li		$8,0xbd012020 # TC0DATA
	li      $9,0x0AE63200 # 100ms
	sw		$9,0($8)

1:
	li		$8,0xbd012028 # TC0CNT
	lw		$9,0($8)
	bgtu	$9,0x100,1b

	addi	$10,$10,1
	bne		$10,$4,2b     # loop $4 times
	
	jr		$31

	.end counter_delay100ms


	
#/*-------------------------------------------------------------------
#**
#* void BlinkLED()
#*
#**-------------------------------------------------------------------
#*/
	.section ".initmem", "ax"
#	.text
	.globl	BlinkLED
	.ent	BlinkLED
BlinkLED:

	# for bi-color LED
	/*********************************
	li		$8,0xbc805000  # PHY LED
	lw		$9,0($8)
	li		$10,0x00080000
	or		$9,$9,$10
	sw		$9,0($8)
	**********************************/

	li		$8,0xbd01200c  # GPIO A:6
	li		$9,0x3f000f00  # configure as GPIO
	sw		$9,0($8)

	li		$8,0xbd012010  # GPIO A:6
	li		$9,0x40000000  # configure as output
	sw		$9,0($8)

	li		$8,0xbc805000  # PHY LED
	lw		$9,0($8)
	li		$10,0xfffb0000 # Mask PHY LED
	and		$9,$9,$10
	li		$10,0x000471c7
	or		$9,$9,$10
	sw		$9,0($8)

	li		$8,0xbd012014  # GPIO LED
	li		$9,0x40000000
	sw		$9,0($8)

	li		$4, 0x19       # 3 sec
	jal		counter_delay100ms

	li		$8,0xbc805000  # PHY LED
	lw		$9,0($8)
	li		$10,0xfffb0000 # Mask PHY LED
	and		$9,$9,$10
	li		$10,0x00048E38
	or		$9,$9,$10
	sw		$9,0($8)

	li		$8,0xbd012014  # GPIO LED
	li		$9,0x00000000
	sw		$9,0($8)

	li		$4, 0x05       # 0.5 sec
	jal		counter_delay100ms

	j		BlinkLED            # infinite loop

	.end BlinkLED


#/*-------------------------------------------------------------------
#**
#* void testSDRAM()
#*
#*  $112 is reserved for $31
#*
#**-------------------------------------------------------------------
#*/
	.section ".initmem", "ax"
#	.text
	.globl	testSDRAM
	.ent	testSDRAM
testSDRAM:
	move	$11,$31

	li		$8,0x80000000   # test lower 2MB SDRAM

2:
	lw		$9,0($8)		# Read data in testing RAM
	not		$9				# NOT the data
	sw		$9,0($8)		# Store the NOTed data back
	lw		$10,0x10($8)	# load a dummy data
	lw		$10,0($8)		# Read again
	beq		$9,$10,1f
	#--- data is not equal, SDRAM is ERROR.
	j		BlinkLED

1:
	#--- data is equal, try next word.
	addi	$8,$8,0x714         # 0x714 is a magic number (for fast scanning the while SDRAM)
	li		$9,0x80200000-0x10  # loop until 0x80200000(2MB)
	bltu	$8,$9,2b

	move	$31,$11
	jr		$31

	.end testSDRAM


#/*-------------------------------------------------------------------
#**
#* void _initmem(void)
#*
#**-------------------------------------------------------------------
#*/
	.section ".initmem", "ax"
#	.text
	.globl	_initmem
	.ent	_initmem
_initmem:
	move	$23,$31				# save return PC

	#--- test chip revision: IS_865XB()
	li		$5,0xbc805104
	lw		$4,0($5)
	andi	$4,0x0000ffff
	li		$5,0x00005788
	beq		$4,$5,1f			# 50B, jump

	#--- configure memory timing (50A)
	#--- configure MCR
	li		$4,0xe2a30000		# default 16-bit, 16MB SDRAM, 2MB FLASH, 25MHz
	li		$5,0xBD013000
	sw		$4,0($5)
	
	li		$4,0x2f2f2f00		# Flash timing
	li		$5,0xBD013004
	sw		$4,0($5)

	li		$4,0x00000463		# SDRAM timing
	li		$5,0xBD013008
	sw		$4,0($5)

	j		2f

1:
	#--- configure memory timing (50B)
	#
	#  An example:
	#      16-bit, 32MB SDRAM, 2MB FLASH, 200MHz
	#  MCR=0xfaa00000
	#
	lw		$4,regMcr0				# MCR
	lw		$5,regMcr0_sdram		# SDRAM
	or		$4, $4, $5
	lw		$5,regMcr0_bank1type	# Bank1 type
	or		$4, $4, $5
	li		$5,0xBD013000
	sw		$4,0($5)
	
	lw		$4,regMcr1		# Flash timing
	li		$5,0xBD013004
	sw		$4,0($5)

	lw		$4,regMcr2		# SDRAM timing
	li		$5,0xBD013008
	sw		$4,0($5)

    #--- configure SDRxCkDly to 4.78ns (SDRAM Rx Clock Delay Control, see 16.5 System Clock Control Register)
    la      $8, 0xbd01204c
    lw      $9, 0($8)
    li      $10, 0xfffffffc
    and     $9, $9, $10
    lw		$10, regMcrD	# SDRxCkDly
    or      $9, $9, $10
    sw      $9, 0($8)
    nop
	
2:
	#--- Louis note: we no longer support this. Please specify 16-bit/32-bit in MCR register.
	#--- test if 32-bit mode works
	# la		$8,0x33cc55aa
	# sw		$8,0xa0000000
	# lw		$9,0xa0000000
	# beq		$8,$9,1f
	#--- 32-bit mode does not work, change to 16-bit mode
	# li		$8,0xffefffff
	# and		$4,$4,$8
	# sw		$4,0($5)
1:

	# jal		testSDRAM			# test if SDRAM is workable
	# nop

	move	$31,$23				# restore return PC
	j		$31
	
	.end _initmem
	
