# 2.59 is needed by GNULib modules
AC_PREREQ([2.59])
AC_INIT(fdisk, 1.2.4, [bug-fdisk@gnu.org])
AC_CONFIG_AUX_DIR(autotools)

AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)
AM_GNU_GETTEXT_VERSION([0.17])
AM_GNU_GETTEXT([external])

AM_MAINTAINER_MODE

dnl set some basic variables
BUILD_DATE="`date '+%D'`"
AC_DEFINE_UNQUOTED(BUILD_DATE, "$BUILD_DATE", [The date of the build.])

# Check for programs.
AC_PROG_AWK
AC_LANG_C
AC_PROG_CC

dnl check for makeinfo
# MAKEINFO=
# AC_CHECK_PROG(MAKEINFO, makeinfo, makeinfo)
# AM_CONDITIONAL([HAVE_MAKEINFO], [test -n "$MAKEINFO"])

# add calls for gnulib
gl_EARLY

AC_PROG_INSTALL
AC_PROG_LN_S

# configure.ac:24: warning: AC_CANONICAL_HOST invoked multiple times (mmm...)
AC_CANONICAL_HOST
case "$host_os" in
	linux*)
		AC_DEFINE([NAMING_LINUX], [1], [Define 1 if this is an OS with Linux naming scheme]) ;;
	freebsd*)
		AC_DEFINE([NAMING_BSD], [1], [Define 1 if this is an OS with BSD naming scheme]) ;;
esac

# Check headers.
# getopt.h removed because we already have it;
AC_CHECK_HEADERS([ctype.h errno.h stdio.h stdarg.h locale.h])
AC_CHECK_HEADERS([string.h stdlib.h unistd.h parted/parted.h])

# Check for library functions.
#AC_CHECK_FUNCS([])

# Check for LFS
AC_SYS_LARGEFILE
CFLAGS="$CFLAGS -D_FILE_OFFSET_BITS=${ac_cv_sys_file_offset_bits}"

dnl libparted
# hack
# LIBS="$LIBS -ldl"
## better hack
XLIBS="$LIBS"
LIBS="$LIBS -ldl"
CHECK_LIBPARTED([1.7.0], ,
	AC_MSG_ERROR([*** libparted >= 1.7.0 not installed - please install first ***]))
AC_SUBST(PARTED_LIBS)

LIBS="$LIBS -lparted"
# 7 July 2009: New version of libparted does not have ped_free
GFDISK_libparted_ped_free=ped_free
AC_MSG_CHECKING([for ped_free in libparted])
AC_LINK_IFELSE(
	[AC_LANG_PROGRAM([[extern void ped_free( );]], [[ped_free(); return 0;]])],
	[AC_MSG_RESULT(yes)], [GFDISK_libparted_ped_free=free; AC_MSG_RESULT([no])])
AC_DEFINE_UNQUOTED([PED_FREE], [$GFDISK_libparted_ped_free], 
	[Defined to `free' in system where libparted does not export ped_free])

LIBS="$XLIBS"

dnl ncurses
dnl CHECK_CURSES defined in m4/check.m4
CHECK_CURSES(have_curses=true , have_curses=false)
AC_SUBST(CURSES_LIBS)

dnl <BEGIN> readline checking

dnl check for libtermcap first
fdisk_have_libtermcap=0
AC_CHECK_HEADERS([termcap.h])
AC_CHECK_LIB( [termcap], [tgetnum], [fdisk_have_libtermcap="termcap"], 
	[AC_CHECK_LIB( [termlib], [tgetnum], [fdisk_have_libtermcap="termlib"], [fdisk_have_libtermcap="false"])])

if test "$fdisk_have_libtermcap" != "false" ; then
   AC_DEFINE( [HAVE_LIBTERMCAP], 1, [Defined to 1 if you have termcap or termlib library])
   LIBS="$LIBS -l$fdisk_have_libtermcap"
else
   AC_DEFINE( [HAVE_LIBTERMCAP], 0, [Defined to 1 if you have termcap or termlib library])
fi

dnl check for readline
fdisk_have_libreadline=0

AC_CHECK_LIB([readline], [readline], [fdisk_have_libreadline=1; LIBS="$LIBS -lreadline";], [fdisk_have_libreadline=0], [])
AC_CHECK_HEADERS([readline/readline.h readline/history.h], [], [fdisk_have_libreadline=0; break;])

AC_CHECK_TYPES([rl_compentry_func_t], [], [], [
#if HAVE_LIBREADLINE
#	include <readline/readline.h>
# include <readline/history.h>
#endif
])

AC_DEFINE_UNQUOTED([HAVE_LIBREADLINE], [$fdisk_have_libreadline], [Defined to 1 if your system have readline library])

dnl <END> readline checking

dnl Check if we have check...

dnl PKG_CHECK_MODULES([CHECK],[check >= 0.8.2], have_check=true, have_check=false)
CHECK_CHECK([0.8.2], have_check=true, have_check=false)
AM_CONDITIONAL(MAKE_TESTS, test x$have_check = xtrue)
AM_CONDITIONAL(MAKE_CFDISK, test x$have_curses = xtrue)

dnl Check how to create the {g,l}fdisk links
AC_MSG_CHECKING([how to create links])
rm -f test00 test01
touch test00
ln -s test00 test01 || :
SYMLINK=""
SYMLINKFROM="fdisk"
if test -L test01; then
	SYMLINK="ln -s"
else
	SYMLINKFROM="${sbindir}/fdisk"
	rm -f test01
	ln test00 test01 || :
	if test -e test01; then
		SYMLINK="ln"
	else
		SYMLINK="cp"
	fi
fi
AC_MSG_RESULT($SYMLINK)
AC_SUBST(SYMLINK)
AC_SUBST(SYMLINKFROM)
rm -f test00 test01

AC_ARG_WITH([gnu-ext], AS_HELP_STRING([--with-gnu-ext],[Enable GNU extensions]),
   AC_DEFINE([GNU_EXT], [1], [Define to 1 if you enable GNU extensions]))

AC_ARG_ENABLE(cfdisk, AS_HELP_STRING([--disable-cfdisk],[Do not compile cfdisk]),
   [case "${enableval}" in
     yes) cfdisk=true ;;
     no)  cfdisk=false ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --enable-cfdisk) ;;
   esac],
   [cfdisk=auto])

if test x$cfdisk = xtrue && test x$have_curses != xtrue; then
	AC_MSG_ERROR([*** curses required for cfdisk ***])
fi

AM_CONDITIONAL(MAKE_CFDISK, test x$cfdisk != xfalse && test x$have_curses = xtrue)

# add for gnulib
gl_INIT

# lib/Makefile is for GNULib
AC_CONFIG_FILES([Makefile src/Makefile doc/Makefile
 tests/Makefile po/Makefile.in m4/Makefile lib/Makefile])

AC_OUTPUT


