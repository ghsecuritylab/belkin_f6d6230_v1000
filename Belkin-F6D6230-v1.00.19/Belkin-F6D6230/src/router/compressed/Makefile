#
# Makefile for Broadcom BCM947XX boards
#
# Copyright (C) 2008, Broadcom Corporation
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.7 2009/03/12 03:00:51 Exp $
#


# Link at 3 MB offset in RAM
TEXT_START	?= 0x80300000

LOADADDR	:= 0x80001000
CROSS_COMPILE	?= mipsel-linux-

OBJCOPY		:= $(CROSS_COMPILE)objcopy -O binary -R .reginfo -R .note -R .comment -R .mdebug -S
OBJCOPYSREC	:= $(CROSS_COMPILE)objcopy -O srec -R .reginfo -R .note -R .comment -R .mdebug -S

vpath %.c $(SRCBASE)/shared
vpath %.S $(SRCBASE)/shared
vpath %.lds.in $(SRCBASE)/shared

ASFLAGS		+= -D__ASSEMBLY__ -DLOADADDR=$(LOADADDR) -march=mips32 -Wa,-mips32 -Wa,--trap
CFLAGS		+= -DLOADADDR=$(LOADADDR) -march=mips32 -Wa,-mips32 -Wa,--trap
# The self-decompresor is standalone, get rid of the linux flags
CFLAGS		:= $(subst -Dlinux,,$(CFLAGS))
CFLAGS		+= -Ulinux -D_MINOSL_ -ffreestanding
ifdef CONFIG_MCOUNT
CFLAGS		:= $(subst -pg,,$(CFLAGS))
endif
CFLAGS		+= -ffunction-sections $(call check_gcc, -fvtable-gc, )

SYSTEM		?= $(LINUXDIR)/vmlinux
OBJECTS		:= boot.o sisdram.o sbsdram.o aisdram.o load.o sflash.o \
		hndmips.o hndchipc.o \
		sbutils.o aiutils.o siutils.o hndpmu.o nicpci.o \
		min_osl.o bcmutils.o sromstubs.o nvramstubs.o bcmstdlib.o

CFLAGS		+= -G 0 -mno-abicalls -fno-pic -pipe -msoft-float
ASFLAGS		+= -G 0 -mno-abicalls -fno-pic -pipe

ifdef CONFIG_SQUASHFS
# Default to lzma
COMPRESS	?= lzma e
CFLAGS		+= -DUSE_LZMA -D_LZMA_NO_SYSTEM_SIZE_T -DBCMLZMA -I$(SRCBASE)/tools/misc/lzma_src/C

else
# Default to bzip2
COMPRESS	?= bzip2 -c

ifneq ($(findstring gzip,$(COMPRESS)),)
CFLAGS		+= -DUSE_GZIP
else
ifneq ($(findstring bzip2,$(COMPRESS)),)
CFLAGS		+= -DUSE_BZIP2
else
COMPRESS	:= cat
endif
endif
endif

CFLAGS		+= -I$(SRCBASE)/include -DBCMDRIVER
ASFLAGS		+= -I$(SRCBASE)/include

all: zImage vmlinuz

ifeq ($(CONFIG_SQUASHFS), y)
all: vmlinuz-lzma
endif

# Don't build dependencies, this may die if $(CC) isn't gcc
dep:

zImage: vmlinux
	$(OBJCOPY) $< $@

# Link the loader and the kernel binary together
vmlinux: vmlinux.lds $(OBJECTS) piggy.o
	$(LD) -static --gc-sections -no-warn-mismatch -T vmlinux.lds -o $@ $(OBJECTS) piggy.o

vmlinux.lds: hndrte.lds.in Makefile
	@sed -e s/TEXT_START/$(TEXT_START)/ \
	     -e s/TARGET_ARCH/mips/ < $< > $@

# Create a linkable version of the (possibly compressed) kernel binary
piggy.o: piggz piggy.lds
	$(LD) -no-warn-mismatch -T piggy.lds -r -o $@ -b binary piggz -b elf32-tradlittlemips

piggy.lds:
	@echo "SECTIONS { .data : { input_len = .; LONG(input_data_end - input_data) input_data = .; *(.data) input_data_end = .; }}" > $@

# Always create a gzipped version named vmlinuz for compatibility
vmlinuz: piggy
	gzip -c9 $< > $@

piggz: piggy
ifdef CONFIG_SQUASHFS
	$(COMPRESS) $< $@
else
	$(COMPRESS) $< > $@
endif

ifdef CONFIG_SQUASHFS
# Create an lzma version named vmlinuz-lzma
vmlinuz-lzma: piggy
	lzma e $<  $@
endif

piggy: $(SYSTEM)
	$(OBJCOPY) $< $@
	
mrproper: clean

clean:
	rm -f vmlinux vmlinuz zImage piggz piggy *.lds *.o
ifeq ($(CONFIG_SQUASHFS), y)
	rm -f vmlinuz-lzma
endif
