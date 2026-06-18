#!/bin/bash

#echo " **** $0 $*" > /dev/console
#env | sed -e "s/^/ $DEVNAME >> /g" > /dev/console
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin
UDEV_HANDLER_LOCK=/var/lock/udev_handler

[[ $DEVTYPE != disk ]] && exit 0
[[ -z $DISK_NO ]] && exit 0

. /usr/local/lib/libbuffalo.sh

until ln -s $UDEV_HANDLER_LOCK $UDEV_HANDLER_LOCK
do
	usleep 200000
done

trap "rm $UDEV_HANDLER_LOCK" EXIT

if [[ $ACTION == add ]]; then
	# update devlink
	sed -i -e "/^_disk${DISK_NO}/d" -e "/\/dev\/disk${DISK_NO}/d" /var/tmp/devlink
	echo "_disk${DISK_NO}=${DEVNAME}" >> /var/tmp/devlink
	echo  "${DEVNAME#/dev/}=/dev/disk${DISK_NO}"  >> /var/tmp/devlink
	backup_devlink
elif [[ $ACTION == remove ]]; then
	/usr/local/bin/service_control.sh media stop
	/usr/local/bin/service_control.sh share stop

	sed -i -e "/^_disk${DISK_NO}/d" -e "/\/dev\/_disk${DISK_NO}/d" /var/tmp/devlink

	/usr/local/bin/service_control.sh media start
	/usr/local/bin/service_control.sh share start
fi
