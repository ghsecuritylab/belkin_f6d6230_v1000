#
# Miscellaneous utilities Makefile
#
# Copyright (C) 2008, Broadcom Corporation
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.24 2007/02/02 02:56:30 Exp $
#

UTILS := epi_ttcp
CFLAGS += -s -O2 -I$(SRCBASE)/include

#ifdef WLCNT
CFLAGS += -DWLCNT
#endif

vpath %.c $(SRCBASE)/tools/misc

UTILS += et

ifneq ($(wildcard $(SRCBASE)/wl/exe),)
CFLAGS += -I$(SRCBASE)/wl/exe
UTILS += wl
vpath %.c $(SRCBASE)/wl/exe $(SRCBASE)/shared
endif

all:

clean:

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install $(UTILS) $(INSTALLDIR)/usr/sbin
	$(STRIP) $(foreach file,$(UTILS),$(INSTALLDIR)/usr/sbin/$(file))
