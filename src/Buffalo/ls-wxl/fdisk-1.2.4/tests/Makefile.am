if MAKE_TESTS
TESTS = \
	check_common check_advanced

noinst_PROGRAMS = \
	check_common check_advanced

check_common_SOURCES = \
	check_common.c \
	functions.c \
	functions.h \
	../src/common.h \
	../src/common.c \
	../src/strlist.h \
	../src/strlist.c

check_advanced_SOURCES = \
	check_advanced.c \
	functions.c \
	functions.h \
	../src/common.h \
	../src/common.c \
	../src/strlist.h \
	../src/strlist.c

check_common_LDADD = $(PARTED_LIBS) $(CHECK_LIBS)
check_advanced_LDADD = $(PARTED_LIBS) $(CHECK_LIBS)
endif MAKE_TESTS