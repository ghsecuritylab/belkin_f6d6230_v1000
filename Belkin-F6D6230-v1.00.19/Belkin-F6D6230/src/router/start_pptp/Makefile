
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include -Wall -I$(LINUXDIR)/include -I$(TOP)/busybox-1.x -I$(SRCBASE)/router/libbcm -Wall -O
LDFLAGS	+= -L$(TOP)/nvram -L$(PLATFORMDIR)/install/nvram/usr/lib -lnvram -L$(TOP)/shared -L$(PLATFORMDIR)/install/shared/usr/lib -lshared \
	-L$(TOP)/libbcmcrypto -L$(INSTALLDIR)/libbcmcrypto/usr/lib -lbcmcrypto -L$(TOP)/libbcm -L$(INSTALLDIR)/libbcm/usr/lib -lbcm

install: 
	install -D start_pptp $(INSTALLDIR)/usr/sbin/start_pptp
	$(STRIP) $(INSTALLDIR)/usr/sbin/start_pptp

all: 

clean:

