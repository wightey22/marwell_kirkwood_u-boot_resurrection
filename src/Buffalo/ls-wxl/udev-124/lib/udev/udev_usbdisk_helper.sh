#!/bin/bash

#echo " * $0 $*" > /dev/console

# diskinfoに対象USBディスクのIDが記録されていればそのディスク番号を返す．
# なければ空いているディスク番号を返す．
# 空いている番号がなければ，現在接続されていないディスク番号を返す

diskNo=$(sed -n -e "s/usb_disk\(.*\)=\"*${ID_SERIAL}\"*/\1/p" /etc/melco/diskinfo)
if [[ $ACTION == add && $DEVTYPE == disk && -n $diskNo && -L /dev/usbdisk${diskNo} ]]; then
	real_device=$(ls -l /dev/usbdisk${diskNo} | awk '{print $11}')
	disk=$(echo $DEVNAME|sed -e 's/[0-9]*//g')
	if [[ $disk != $real_device ]]; then
		diskNo=
	fi
fi

if [[ -z $diskNo ]] && [[ $DEVTYPE == partition ]]; then
	sdx=$(echo $DEVNAME|sed -e 's/[0-9]*//g')
	diskNo=$(ls -l /dev/usbdisk* | sed -n -e "/$sdx/ s/.*\/dev\/usbdisk\([0-9]*\)\s.*/\1/p")
fi

. /etc/nas_feature
if [[ -z $diskNo ]]; then
	. /etc/melco/diskinfo
	i=1
	while [[ $i -le $MAX_USBDISK_NUM ]]
	do
		if [[ -z $(eval echo '$'usb_disk${i}) ]]; then
			diskNo=$i
			break
		fi
		i=$((i+1))
	done
fi

if [[ -z $diskNo ]]; then
	i=1
	while :
	do
		if [[ ! -L /dev/usbdisk${i} ]]; then
			diskNo=$i
			break
		fi
		i=$((i+1))
	done
fi

echo "USBDISK_NO=$diskNo"
