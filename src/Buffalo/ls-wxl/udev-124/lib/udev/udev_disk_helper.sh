#!/bin/bash

PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin
#echo " ***** $0 $*" > /dev/console
#env | sed -e "s/^/ $DEVNAME >> /g" > /dev/console

host=$(echo $ID_PATH|awk -F: '{print $1}'|sed -e 's/scsi-//')
bus=$(echo $ID_PATH|awk -F: '{print $2}')
target=$(echo $ID_PATH|awk -F: '{print $3}')
lun=$(echo $ID_PATH|awk -F: '{print $4}')

if [[ $host == 0 && $bus == 0 && $target == 0 && $lun == 0 ]]; then
	diskNo=1
elif [[ $host != 0 ]]; then
	diskNo=$((host + 1))
elif [[ $target != 0 ]]; then
	diskNo=$((target + 1))
fi

echo "DISK_NO=$diskNo"
