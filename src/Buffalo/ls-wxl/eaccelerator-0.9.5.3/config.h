/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.in by autoheader.  */

/* Whether to build eaccelerator as dynamic module */
#define COMPILE_DL_EACCELERATOR 1

/* Undef when you want to enable eaccelerator debug code */
/* #undef DEBUG */

/* The userid eAccelerator will be running under. */
#define EA_USERID 0

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define if you like to use eAccelerator */
#define HAVE_EACCELERATOR 1

/* Define if you have the <ext/session/php_session.h> header file. */
#define HAVE_EXT_SESSION_PHP_SESSION_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define if ou have mprotect function */
#define HAVE_MPROTECT 1

/* Define to 1 if you have the <sched.h> header file. */
#define HAVE_SCHED_H 1

/* Define if ou have sched_yield function */
#define HAVE_SCHED_YIELD 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define if you have semun union in sys/sem.h */
/* #undef HAVE_UNION_SEMUN */

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* If you want eAccelerator to retain doc-comments in internal php structures
   (meta-programming) */
/* #undef INCLUDE_DOC_COMMENTS */

/* Define if you like to use fcntl based semaphores */
/* #undef MM_SEM_FCNTL */

/* Define if you like to use flock based semaphores */
/* #undef MM_SEM_FLOCK */

/* Define if you like to use sysvipc based semaphores */
/* #undef MM_SEM_IPC */

/* Define if you like to use posix based semaphores */
/* #undef MM_SEM_POSIX */

/* Define if you like to use pthread based semaphores */
/* #undef MM_SEM_PTHREAD */

/* Define if you like to use spinlock based semaphores */
#define MM_SEM_SPINLOCK 1

/* Define if you like to use sysvipc based shared memory */
/* #undef MM_SHM_IPC */

/* Define if you like to use anonymous mmap based shared memory */
#define MM_SHM_MMAP_ANON 1

/* Define if you like to use mmap on temporary file shared memory */
/* #undef MM_SHM_MMAP_FILE */

/* Define if you like to use posix mmap based shared memory */
/* #undef MM_SHM_MMAP_POSIX */

/* Define if you like to use mmap on /dev/zero based shared memory */
/* #undef MM_SHM_MMAP_ZERO */

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define if you like to use eAccelerator content cachin API */
/* #undef WITH_EACCELERATOR_CONTENT_CACHING */

/* Define if you like to release eAccelerator resources on PHP crash */
#define WITH_EACCELERATOR_CRASH_DETECTION 1

/* Define if you like to explore Zend bytecode */
/* #undef WITH_EACCELERATOR_DISASSEMBLER */

/* Define if you like to use eAccelerator enoder */
#define WITH_EACCELERATOR_ENCODER 1

/* Define if you want the information functions */
#define WITH_EACCELERATOR_INFO 1

/* Define if you like to load files encoded by eAccelerator encoder */
#define WITH_EACCELERATOR_LOADER 1

/* Define if you like to use peephole opcode optimization */
#define WITH_EACCELERATOR_OPTIMIZER 1

/* Define if you like to use eAccelerator session handlers to store session's
   information in shared memory */
/* #undef WITH_EACCELERATOR_SESSIONS */

/* Define if you like to use the eAccelerator functions to store keys in
   shared memory */
/* #undef WITH_EACCELERATOR_SHM */

/* Undef if you don't wan't to use inodes to determine hash keys */
#define WITH_EACCELERATOR_USE_INODE 1
