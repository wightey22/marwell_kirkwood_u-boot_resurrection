PACKAGE = @PACKAGE@
VERSION = @VERSION@

top_srcdir = @top_srcdir@
subdir = src
datadir = @datadir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
LIBS = @LIBINTL@ @LIBS@

AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib
LDADD = $(top_builddir)/lib/libgnu.a

sbin_PROGRAMS =  fdisk

fdisk_SOURCES = main.c		\
		command.c	\
		command.h	\
		fdisk.c  	\
		common.c	\
		common.h	\
		hacks.c		\
		hacks.h		\
		strlist.c	\
		strlist.h	\
		ui.c		\
		ui.h

fdisk_LDADD = $(PARTED_LIBS)



if MAKE_CFDISK
sbin_PROGRAMS += cfdisk

cfdisk_SOURCES = cfdisk.c	\
		common.c	\
		common.h	\
		strlist.c	\
		strlist.h	\
		hacks.c		\
		hacks.h		\
		sys_types.h

cfdisk_LDADD = $(PARTED_LIBS) $(CURSES_LIBS)

endif MAKE_CFDISK

install-exec-local:
	@SYMLINK@ @SYMLINKFROM@ $(DESTDIR)/$(sbindir)/lfdisk
	@SYMLINK@ @SYMLINKFROM@ $(DESTDIR)/$(sbindir)/gfdisk

install-exec-am: install-sbinPROGRAMS install-exec-local
