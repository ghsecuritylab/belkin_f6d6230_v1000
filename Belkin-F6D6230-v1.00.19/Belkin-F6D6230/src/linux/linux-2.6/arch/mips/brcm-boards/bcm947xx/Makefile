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
# $Id: Makefile,v 1.4 2009/07/27 03:44:16 Exp $
#

SHARED := ../../../../../../shared

BCM947XX_OBJS = $(addprefix $(SHARED)/, hndmips.o hndchipc.o linux_gpio.o sflash.o)
BCM947XX_OBJS += $(addprefix $(SHARED)/nvram/, nvram.o)

EXTRA_CFLAGS += -DLINUX_HYBRID_POSTPROCESSING_REMOVAL

ifeq ($(CONFIG_PCI),y)
BCM947XX_OBJS += $(addprefix $(SHARED)/, hndpci.o) pcibios.o
endif

ifeq ($(CONFIG_WAPI),y)
EXTRA_CFLAGS += -DBCMWAPI_WAI
endif

BCM947XX_OBJS += setup.o prom.o time.o irq.o perfcntr.o

bcm947xx-objs += $(BCM947XX_OBJS)
obj-y += bcm947xx.o
