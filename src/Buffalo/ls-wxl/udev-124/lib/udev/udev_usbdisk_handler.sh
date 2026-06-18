#!/bin/bash

#echo " * $0 $*" > /dev/console

PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin
UDEV_HANDLER_LOCK=/var/lock/udev_handler
DEVLINK_LOCK=/var/lock/devlink
LOCK_TIMEOUT=10
DC=/usr/local/sbin/directcopy.pyc
PERL=/usr/bin/perl
PYTHON=/usr/bin/python

. /etc/nas_feature
. /usr/local/lib/libbuffalo.sh

until ln -s $UDEV_HANDLER_LOCK $UDEV_HANDLER_LOCK
do
	usleep 200000
done

trap "rm $UDEV_HANDLER_LOCK" EXIT

[[ -z $USBDISK_NO ]] && exit 1
[[ $USBDISK_NO -gt $MAX_USBDISK_NUM ]] && exit 1
PART=${DEVNAME#/dev/sd[a-z]}


if [[ $ACTION == add ]] && [[ $DEVTYPE == disk ]]; then
	# update diskinfo
	if grep -q "^usb_disk${USBDISK_NO}=" /etc/melco/diskinfo; then
		sed -i -e "s|^usb_disk${USBDISK_NO}=.*|usb_disk${USBDISK_NO}=\"${ID_SERIAL}\"|g;" /etc/melco/diskinfo
	else
		echo "usb_disk${USBDISK_NO}=\"${ID_SERIAL}\"" >> /etc/melco/diskinfo
	fi

	# update devlink
	until ln -s $DEVLINK_LOCK $DEVLINK_LOCK
	do
		sleep 1
	done
	sed -i -e "/usbdisk${USBDISK_NO}/d" /var/tmp/devlink
	echo "usbdisk${USBDISK_NO}=${DEVNAME}" >> /var/tmp/devlink
	echo  "${DEVNAME#/dev/}=/dev/usbdisk${USBDISK_NO}"  >> /var/tmp/devlink
	backup_devlink
	rm $DEVLINK_LOCK

	# update usb_list
	record="${ID_SERIAL}<>1<>/dev/usbdisk${USBDISK_NO}<>${ID_VENDOR}<>${ID_MODEL}<>${ID_MODEL}<>1;"
	if grep -q "${ID_SERIAL}" /var/tmp/usb_list; then
		sed -i -e "s|.*${ID_SERIAL}.*|$record|g;" /var/tmp/usb_list
	else
		echo "$record" >> /var/tmp/usb_list
	fi
elif [[ $ACTION == add ]] && [[ $USBDISK_NO -le $MAX_USBDISK_NUM ]] \
    && [[ $PART == 1 || $PART == 5 ]] && [[ $ID_FS_USAGE == filesystem ]]; then
	[[ -f /var/lock/disk ]] && exit 1

	if grep -q "/mnt/usbdisk${USBDISK_NO}" /proc/mounts; then
		exit 1
	fi

	mount_usbdisk $USBDISK_NO || exit $?
	if [[ -x $PERL && $SUPPORT_WEBAXS == 1 ]]; then
		$PERL /usr/local/bin/WebAxs_MountUsbDisk.pl usbdisk${USBDISK_NO}
	fi

	cpu_status=$(</proc/buffalo/cpu_status)
	if [[ $cpu_status == normal_state ]]; then
		if [[ -x $PYTHON && $SUPPORT_DIRECT_COPY == on ]]; then
			$PYTHON $DC stop
			$PYTHON $DC --background --srcdir /mnt/usbdisk${USBDISK_NO}
		fi
		/usr/local/bin/service_control.sh share restart
		/usr/local/bin/service_control.sh media restart
	fi
elif [[ $ACTION == remove ]] && [[ $DEVTYPE == disk ]]; then
	/usr/local/bin/service_control.sh media stop
	/usr/local/bin/service_control.sh share stop

	/usr/local/lib/libdisk.sh umount /mnt/usbdisk${USBDISK_NO}
	if [[ -x $PYTHON && $SUPPORT_DIRECT_COPY == on ]]; then
		$PYTHON $DC stop
	fi

	if [[ -x $PERL && $SUPPORT_WEBAXS == 1 ]]; then
		$PERL /usr/local/bin/WebAxs_UnmoutUsbDisk.pl usbdisk${USBDISK_NO}
	fi

	until ln -s $DEVLINK_LOCK $DEVLINK_LOCK
	do
		sleep 1
	done
	sed -i -e "/usbdisk${USBDISK_NO}/d" /var/tmp/devlink
	rm $DEVLINK_LOCK

	/usr/local/bin/service_control.sh media start
	/usr/local/bin/service_control.sh share start
fi
