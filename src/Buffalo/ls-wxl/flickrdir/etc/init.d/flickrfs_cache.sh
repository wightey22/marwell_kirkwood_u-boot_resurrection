#!/bin/sh
. /etc/melco/flickrfs/flickrfs

PRIORITY=-0
FLICKRFS_CACHE_LOCK_FILE=/var/run/flickrfs_cache.pid
RSYNC_LOCK_FILE=/var/run/flickrfs_rsync.pid
FLICKRFS_SYNC=/etc/init.d/flickrfs.sh
FLICKRFS_FIND_LOCK_FILE=/var/run/flickrfs_find.pid
SPOOL_DIR=`libbuffalo_bin GetSpoolDir`
WATCH_DIR=${SPOOL_DIR}/flickr
#MOUNT_POINT=$1
if [ $1 ne "" ]; then
	MOUNT_POINT=$1	
else
	MOUNT_POINT=$mntpoint
fi

PIDFileCheck()
{
	if [ -f FLICKRFS_CACHE_LOCK_FILE ]; then
		echo "$0 already running." >&2
		return 1
	fi

	echo $$ > $FLICKRFS_CACHE_LOCK_FILE

	trap "rm $FLICKRFS_CACHE_LOCK_FILE"	EXIT
}

process()
{
	echo "caching system."
        while true                                                        
	do      
		sleep 60
		if [ -s ${FLICKRFS_FIND_LOCK_FILE} ]; then 
			local pid=`cat ${FLICKRFS_FIND_LOCK_FILE}`
			echo "your find pid is $pid"
			kill -CONT ${pid}
			local isAlive=$?
			if [ $isAlive = 1 ]; then
				rm ${FLICKRFS_FIND_LOCK_FILE}
			fi
		fi
		if [ ! -f ${FLICKRFS_FIND_LOCK_FILE} ]; then
			echo "caching system starting..."
	                nice $PRIORITY find $MOUNT_POINT -type f -maxdepth 3 \( -name \*.jpg -o -name \*.JPG -o -name \*.jpeg -o -name \*.JPEG -o -name \*.gif -o -name \*.GIF -o -name \*.tif -o -name \*.TIF -o -name \*.tiff -o -name \*.TIFF -o -name \*.bmp -o -name \*.BMP \) ! -name "*.meta" -print -exec cat {} > /dev/null \; > /dev/null 2>&1 &
			echo $! > ${FLICKRFS_FIND_LOCK_FILE}
		fi
		sleep 240
	done 
}

PIDFileCheck || exit 1
process
