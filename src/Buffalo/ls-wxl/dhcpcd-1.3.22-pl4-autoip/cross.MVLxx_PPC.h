CROSS_COMPILE=ppc_82xx-
export CC     = $(CROSS_COMPILE)gcc
export AR     = $(CROSS_COMPILE)ar
export RANLIB = $(CROSS_COMPILE)ranlib
export STRIP  = $(CROSS_COMPILE)strip
export LD     = $(CROSS_COMPILE)ld
export NM     = $(CROSS_COMPILE)nm
export AS     = $(CROSS_COMPILE)as
