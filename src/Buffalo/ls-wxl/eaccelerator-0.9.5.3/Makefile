srcdir = /home/bart/development/eaccelerator/branches/0.9.5
builddir = /home/bart/development/eaccelerator/branches/0.9.5
top_srcdir = /home/bart/development/eaccelerator/branches/0.9.5
top_builddir = /home/bart/development/eaccelerator/branches/0.9.5
EGREP = /bin/grep -E
SED = /bin/sed
CONFIGURE_COMMAND = './configure'
SHLIB_SUFFIX_NAME = so
SHLIB_DL_SUFFIX_NAME = so
RE2C = exit 0;
AWK = gawk
shared_objects_eaccelerator = eaccelerator.lo optimize.lo encoder.lo loader.lo opcodes.lo content.lo mm.lo session.lo shm.lo debug.lo cache.lo ea_restore.lo ea_store.lo ea_info.lo ea_dasm.lo
PHP_PECL_EXTENSION = eaccelerator
PHP_MODULES = $(phplibdir)/eaccelerator.la
all_targets = $(PHP_MODULES)
install_targets = install-modules install-headers
prefix = /usr
exec_prefix = $(prefix)
libdir = ${exec_prefix}/lib
prefix = /usr
phplibdir = /home/bart/development/eaccelerator/branches/0.9.5/modules
phpincludedir = /usr/include/php
CC = gcc
CFLAGS = -g -O2
CFLAGS_CLEAN = $(CFLAGS)
CPP = gcc -E
CPPFLAGS = -DHAVE_CONFIG_H
CXX = g++
CXXFLAGS = -g -O2
EXTENSION_DIR = /usr/lib/php/modules
PHP_EXECUTABLE = /usr/bin/php
EXTRA_LDFLAGS =
EXTRA_LIBS =
INCLUDES = -I/usr/include/php -I/usr/include/php/main -I/usr/include/php/TSRM -I/usr/include/php/Zend -I/usr/include/php/ext -I/usr/include/php/ext/date/lib
LFLAGS =
LDFLAGS =
SHARED_LIBTOOL =
LIBTOOL = $(SHELL) $(top_builddir)/libtool
SHELL = /bin/sh
INSTALL_HEADERS =
mkinstalldirs = $(top_srcdir)/build/shtool mkdir -p
INSTALL = $(top_srcdir)/build/shtool install -c
INSTALL_DATA = $(INSTALL) -m 644

DEFS = -DPHP_ATOM_INC -I$(top_builddir)/include -I$(top_builddir)/main -I$(top_srcdir)
COMMON_FLAGS = $(DEFS) $(INCLUDES) $(EXTRA_INCLUDES) $(CPPFLAGS) $(PHP_FRAMEWORKPATH)

all: $(all_targets) 
	@echo
	@echo "Build complete."
	@echo "Don't forget to run 'make test'."
	@echo
	
build-modules: $(PHP_MODULES)

libphp5.la: $(PHP_GLOBAL_OBJS) $(PHP_SAPI_OBJS)
	$(LIBTOOL) --mode=link $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -rpath $(phptempdir) $(EXTRA_LDFLAGS) $(LDFLAGS) $(PHP_RPATHS) $(PHP_GLOBAL_OBJS) $(PHP_SAPI_OBJS) $(EXTRA_LIBS) $(ZEND_EXTRA_LIBS) -o $@
	-@$(LIBTOOL) --silent --mode=install cp libphp5.la $(phptempdir)/libphp5.la >/dev/null 2>&1

libs/libphp5.bundle: $(PHP_GLOBAL_OBJS) $(PHP_SAPI_OBJS)
	$(CC) $(MH_BUNDLE_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(PHP_GLOBAL_OBJS:.lo=.o) $(PHP_SAPI_OBJS:.lo=.o) $(PHP_FRAMEWORKS) $(EXTRA_LIBS) $(ZEND_EXTRA_LIBS) -o $@ && cp $@ libs/libphp5.so

install: $(all_targets) $(install_targets)

install-sapi: $(OVERALL_TARGET)
	@echo "Installing PHP SAPI module:       $(PHP_SAPI)"
	-@$(mkinstalldirs) $(INSTALL_ROOT)$(bindir)
	-@if test ! -r $(phptempdir)/libphp5.$(SHLIB_DL_SUFFIX_NAME); then \
		for i in 0.0.0 0.0 0; do \
			if test -r $(phptempdir)/libphp5.$(SHLIB_DL_SUFFIX_NAME).$$i; then \
				$(LN_S) $(phptempdir)/libphp5.$(SHLIB_DL_SUFFIX_NAME).$$i $(phptempdir)/libphp5.$(SHLIB_DL_SUFFIX_NAME); \
				break; \
			fi; \
		done; \
	fi
	@$(INSTALL_IT)

install-modules: build-modules
	@test -d modules && \
	$(mkinstalldirs) $(INSTALL_ROOT)$(EXTENSION_DIR)
	@echo "Installing shared extensions:     $(INSTALL_ROOT)$(EXTENSION_DIR)/"
	@rm -f modules/*.la >/dev/null 2>&1
	@$(INSTALL) modules/* $(INSTALL_ROOT)$(EXTENSION_DIR)

install-headers:
	-@if test "$(INSTALL_HEADERS)"; then \
		for i in `echo $(INSTALL_HEADERS)`; do \
			i=`$(top_srcdir)/build/shtool path -d $$i`; \
			paths="$$paths $(INSTALL_ROOT)$(phpincludedir)/$$i"; \
		done; \
		$(mkinstalldirs) $$paths && \
		echo "Installing header files:          $(INSTALL_ROOT)$(phpincludedir)/" && \
		for i in `echo $(INSTALL_HEADERS)`; do \
			if test "$(PHP_PECL_EXTENSION)"; then \
				src=`echo $$i | $(SED) -e "s#ext/$(PHP_PECL_EXTENSION)/##g"`; \
			else \
				src=$$i; \
			fi; \
			if test -f "$(top_srcdir)/$$src"; then \
				$(INSTALL_DATA) $(top_srcdir)/$$src $(INSTALL_ROOT)$(phpincludedir)/$$i; \
			elif test -f "$(top_builddir)/$$src"; then \
				$(INSTALL_DATA) $(top_builddir)/$$src $(INSTALL_ROOT)$(phpincludedir)/$$i; \
			else \
				(cd $(top_srcdir)/$$src && $(INSTALL_DATA) *.h $(INSTALL_ROOT)$(phpincludedir)/$$i; \
				cd $(top_builddir)/$$src && $(INSTALL_DATA) *.h $(INSTALL_ROOT)$(phpincludedir)/$$i) 2>/dev/null || true; \
			fi \
		done; \
	fi
	
install-su: install-pear

test: all 
	-@if test ! -z "$(PHP_EXECUTABLE)" && test -x "$(PHP_EXECUTABLE)"; then \
		TEST_PHP_EXECUTABLE=$(PHP_EXECUTABLE) \
		TEST_PHP_SRCDIR=$(top_srcdir) \
		CC="$(CC)" \
			$(PHP_EXECUTABLE) -d 'open_basedir=' -d 'safe_mode=0' -d 'output_buffering=0' -d 'memory_limit=-1' $(top_srcdir)/run-tests.php -d 'extension_dir=modules/' -d `( . $(PHP_MODULES) ; echo extension=$$dlname)` tests/; \
	elif test ! -z "$(SAPI_CLI_PATH)" && test -x "$(SAPI_CLI_PATH)"; then \
		TEST_PHP_EXECUTABLE=$(top_builddir)/$(SAPI_CLI_PATH) \
		TEST_PHP_SRCDIR=$(top_srcdir) \
		CC="$(CC)" \
			$(top_builddir)/$(SAPI_CLI_PATH) -d 'open_basedir=' -d 'safe_mode=0' -d 'output_buffering=0' -d 'memory_limit=-1' $(top_srcdir)/run-tests.php -n $(TESTS); \
	else \
		echo "ERROR: Cannot run tests without CLI sapi."; \
	fi

clean:
	find . -name \*.lo -o -name \*.o | xargs rm -f
	find . -name \*.la -o -name \*.a | xargs rm -f 
	find . -name \*.so | xargs rm -f
	find . -name .libs -a -type d|xargs rm -rf
	rm -f libphp5.la $(SAPI_CLI_PATH) $(OVERALL_TARGET) modules/* libs/*

distclean: clean
	rm -f config.cache config.log config.status Makefile.objects Makefile.fragments libtool main/php_config.h stamp-h php5.spec sapi/apache/libphp5.module buildmk.stamp
	$(EGREP) define'.*include/php' $(top_srcdir)/configure | $(SED) 's/.*>//'|xargs rm -f
	find . -name Makefile | xargs rm -f

.PHONY: all clean install distclean test
.NOEXPORT:
eaccelerator.lo: /home/bart/development/eaccelerator/branches/0.9.5/eaccelerator.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/bart/development/eaccelerator/branches/0.9.5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/bart/development/eaccelerator/branches/0.9.5/eaccelerator.c -o eaccelerator.lo 
optimize.lo: /home/bart/development/eaccelerator/branches/0.9.5/optimize.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/bart/development/eaccelerator/branches/0.9.5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/bart/development/eaccelerator/branches/0.9.5/optimize.c -o optimize.lo 
encoder.lo: /home/bart/development/eaccelerator/branches/0.9.5/encoder.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/bart/development/eaccelerator/branches/0.9.5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/bart/development/eaccelerator/branches/0.9.5/encoder.c -o encoder.lo 
loader.lo: /home/bart/development/eaccelerator/branches/0.9.5/loader.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/bart/development/eaccelerator/branches/0.9.5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/bart/development/eaccelerator/branches/0.9.5/loader.c -o loader.lo 
opcodes.lo: /home/bart/development/eaccelerator/branches/0.9.5/opcodes.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/bart/development/eaccelerator/branches/0.9.5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/bart/development/eaccelerator/branches/0.9.5/opcodes.c -o opcodes.lo 
content.lo: /home/bart/development/eaccelerator/branches/0.9.5/content.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/bart/development/eaccelerator/branches/0.9.5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/bart/development/eaccelerator/branches/0.9.5/content.c -o content.lo 
mm.lo: /home/bart/development/eaccelerator/branches/0.9.5/mm.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/bart/development/eaccelerator/branches/0.9.5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/bart/development/eaccelerator/branches/0.9.5/mm.c -o mm.lo 
session.lo: /home/bart/development/eaccelerator/branches/0.9.5/session.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/bart/development/eaccelerator/branches/0.9.5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/bart/development/eaccelerator/branches/0.9.5/session.c -o session.lo 
shm.lo: /home/bart/development/eaccelerator/branches/0.9.5/shm.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/bart/development/eaccelerator/branches/0.9.5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/bart/development/eaccelerator/branches/0.9.5/shm.c -o shm.lo 
debug.lo: /home/bart/development/eaccelerator/branches/0.9.5/debug.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/bart/development/eaccelerator/branches/0.9.5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/bart/development/eaccelerator/branches/0.9.5/debug.c -o debug.lo 
cache.lo: /home/bart/development/eaccelerator/branches/0.9.5/cache.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/bart/development/eaccelerator/branches/0.9.5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/bart/development/eaccelerator/branches/0.9.5/cache.c -o cache.lo 
ea_restore.lo: /home/bart/development/eaccelerator/branches/0.9.5/ea_restore.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/bart/development/eaccelerator/branches/0.9.5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/bart/development/eaccelerator/branches/0.9.5/ea_restore.c -o ea_restore.lo 
ea_store.lo: /home/bart/development/eaccelerator/branches/0.9.5/ea_store.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/bart/development/eaccelerator/branches/0.9.5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/bart/development/eaccelerator/branches/0.9.5/ea_store.c -o ea_store.lo 
ea_info.lo: /home/bart/development/eaccelerator/branches/0.9.5/ea_info.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/bart/development/eaccelerator/branches/0.9.5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/bart/development/eaccelerator/branches/0.9.5/ea_info.c -o ea_info.lo 
ea_dasm.lo: /home/bart/development/eaccelerator/branches/0.9.5/ea_dasm.c
	$(LIBTOOL) --mode=compile $(CC)  -I. -I/home/bart/development/eaccelerator/branches/0.9.5 $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS)  -c /home/bart/development/eaccelerator/branches/0.9.5/ea_dasm.c -o ea_dasm.lo 
$(phplibdir)/eaccelerator.la: ./eaccelerator.la
	$(LIBTOOL) --mode=install cp ./eaccelerator.la $(phplibdir)

./eaccelerator.la: $(shared_objects_eaccelerator) $(EACCELERATOR_SHARED_DEPENDENCIES)
	$(LIBTOOL) --mode=link $(CC) $(COMMON_FLAGS) $(CFLAGS_CLEAN) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ -export-dynamic -avoid-version -prefer-pic -module -rpath $(phplibdir) $(EXTRA_LDFLAGS) $(shared_objects_eaccelerator) $(EACCELERATOR_SHARED_LIBADD)

