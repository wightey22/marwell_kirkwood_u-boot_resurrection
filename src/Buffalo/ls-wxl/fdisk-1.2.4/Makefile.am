PACKAGE = @PACKAGE@
VERSION = @VERSION@

# lib for GNULib
SUBDIRS = m4 po src doc lib

if MAKE_TESTS
SUBDIRS += tests
endif MAKE_TESTS

EXTRA_DIST = autotools/config.rpath \
  autotools/config.rpath \
  ABOUT-NLS AUTHORS THANKS ChangeLog NEWS README fdisk.spec

ACLOCAL_AMFLAGS = -I m4
