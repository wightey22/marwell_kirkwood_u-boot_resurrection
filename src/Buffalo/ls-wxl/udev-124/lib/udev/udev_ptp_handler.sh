#!/bin/sh

export PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin
GPHOTO2=/usr/bin/gphoto2
AWK=/usr/bin/awk
DIRECTCOPY=/usr/local/sbin/directcopy.pyc
PYTHON=/usr/bin/python

bus_dev=$($GPHOTO2 --auto-detect | $AWK -F: '/:/ {print $2}')
if [[ -z $bus_dev ]]; then
	exit
fi

bus_num=$(echo $bus_dev | $AWK -F, '{print $1}')
dev_num=$(echo $bus_dev | $AWK -F, '{print $2}')

if [[ $BUSNUM == $bus_num && $DEVNUM == $dev_num ]]; then
	exec $PYTHON $DIRECTCOPY --background --ptp
fi
