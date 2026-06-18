#!/bin/sh

[ -e /etc/udev/udev.conf ] && . /etc/udev/udev.conf

cleanup()
{
	# fail more gracely and not leave udevd running
	start-stop-daemon --stop --exec /sbin/udevd
	sleep 1
	umount /dev
	exit 1
}

mount_dev_directory()
{
	echo "Mounting /dev"
	mount -n -t tmpfs -o "exec,nosuid,mode=0755,size=10M" udev /dev
}

seed_dev()
{
	# Seed /dev with some things that we know we need

	# creating /dev/console and /dev/tty1 to be able to write
	# to $CONSOLE with/without bootsplash before udevd creates it
	[ -c /dev/console ] || mknod /dev/console c 5 1
	[ -c /dev/tty1 ] || mknod /dev/tty1 c 4 1

	# udevd will dup its stdin/stdout/stderr to /dev/null
	# and we do not want a file which gets buffered in ram
	[ -c /dev/null ] || mknod /dev/null c 1 3

	# copy over any persistant things
	if [ -d /lib/udev/devices ]; then
		cp -RPp /lib/udev/devices/* /dev 2>/dev/null
	fi

	# Not provided by sysfs but needed
	ln -snf /proc/self/fd /dev/fd
	ln -snf fd/0 /dev/stdin
	ln -snf fd/1 /dev/stdout
	ln -snf fd/2 /dev/stderr
	[ -e /proc/kcore ] && ln -snf /proc/kcore /dev/core

	# Create problematic directories
	mkdir -p /dev/pts /dev/shm

	return 0
}

check_group()
{
	for grp in usb video uucp kmem disk floppy cdrom tape audio
	do
		if ! grep -q "^${grp}:" /etc/group; then
			groupadd --system ${grp}
		fi
	done
}

disable_hotplug_agent()
{
	if [ -e /proc/sys/kernel/hotplug ]; then
		echo "" >/proc/sys/kernel/hotplug
	fi
}

enable_hotplug_agent()
{
	if [ -e /proc/sys/kernel/hotplug ]; then
		echo "/sbin/hotplug" >/proc/sys/kernel/hotplug
	fi
}

root_link()
{
	/lib/udev/write_root_link_rule
}

start_udevd()
{
	echo "Starting udevd"
	start-stop-daemon --start --exec /sbin/udevd -- --daemon
}

populate_dev()
{
	udevadm control --env do_not_run_plug_service=1
	udevadm trigger --attr-match=dev
	udevadm trigger --subsystem-match=net
	echo "Waiting for uevents to be processed"
	udevadm settle --timeout=10
	udevadm control --env do_not_run_plug_service=
	return 0
}

check_udev_works()
{
	# should exist on every system, else udev failed
	if [ ! -e /dev/zero ]; then
		echo "Assuming udev failed somewhere, as /dev/zero does not exist."
		return 1
	fi
	echo "" > /dev/console || (cd /dev; rm console; ln -s null console)
	[ -c /dev/ttyS0 ] || (cd /dev; ln -s null ttyS0)

	return 0
}


start()
{
	mount_dev_directory || cleanup
	seed_dev
	check_group
	root_link
	disable_hotplug_agent
	
	start_udevd || cleanup
	populate_dev || cleanup
	check_udev_works || cleanup

#	enable_hotplug_agent
	exit 0
}

stop()
{
    cleanup
}

case $1 in
    start)
	start
	;;
    stop)
	stop
	;;
    *)
	
esac
