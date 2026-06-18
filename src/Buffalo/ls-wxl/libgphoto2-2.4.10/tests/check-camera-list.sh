#!/bin/sh
# "make installcheck" testcase:
# Lists all cameras found by libgphoto2.
# Fails if no cameras are found.

set -e

debug=:
#debug=false

PACKAGE_TARNAME="${PACKAGE_TARNAME-"libgphoto2"}"
prefix="${prefix-"/usr"}"
exec_prefix="${exec_prefix-"${prefix}"}"
libdir="${libdir-"${exec_prefix}/lib"}"
libexecdir="${libexecdir-"${exec_prefix}/libexec"}"
camlibdir="${camlibdir-"${libdir}/libgphoto2/2.4.10"}"
CAMLIBS="${DESTDIR}${camlibdir}"
export CAMLIBS
LD_LIBRARY_PATH="${DESTDIR}/${libdir}${LD_LIBRARY_PATH+:${LD_LIBRARY_PATH}}"
export LD_LIBRARY_PATH


if test -d "${CAMLIBS}"; then :; else
    echo "camlibs directory '${CAMLIBS}' does not exist"
    exit 13
fi

if "$debug"; then
    echo "====================="
    pwd
    echo "camlibdir=$camlibdir"
    echo "libdir=$libdir"
    echo "DESTDIR=$DESTDIR"
    echo "CAMLIBS=$CAMLIBS"
    echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
    echo "#####################"
fi

set -x
./test-camera-list
