.SUFFIXES: .do

include path.h
include cross.h

INSTALLDIR = ./img

#CROSS_COMPILE = ppc_82xx-
#CC = $(CROSS_COMPILE)gcc
#STRIP = $(CROSS_COMPILE)strip


PROGRAM = dhcpcd


SRCS = peekfd.c udpipgen.c buildmsg.c signals.c arp.c dhcpconfig.c \
	client.c cache.c autoip.c dhcpcd.c
OBJS = ${SRCS:.c=.o}
DOBJS = ${SRCS:.c=.do}

CFLAGS = -DBUFFALO_PLATFORM -O2 -fomit-frame-pointer -Wall -I/usr/local/armel-2.0/include
LDFLAGS = -s -O2

LIBS =

all: $(OBJS) 
	$(CC) $(LDFLAGS) -o $(PROGRAM) $(OBJS) $(LIBS)

debug: $(DOBJS)
	$(LD) $(LDFLAGS) -g -o $(PROGRAM) $(DOBJS) $(LIBS)

clean:
	rm -rf *.o *.BAK $(PROGRAM)

install: $(PROGRAM)
	install -d $(INSTALLDIR)/usr/sbin
	install $(PROGRAM) $(INSTALLDIR)/sbin
	$(STRIP) $(INSTALLDIR)/sbin/$(PROGRAM)

.c.do:
	$(CC) $(CFLAGS) -g $< -o ${*F}.do

.c.o:
	$(CC) $(CFLAGS) -c $<

