#!/bin/sh

PRIORITY=-0
FLICKRFS_DIR=/usr/local/webfuse/flickrfs/flickrfs
FLICKRFS_DIR_R=/usr/local/webfuse/flickrfs/
FUSERMOUNT_DIR=/usr/local/bin
FLICKRFS_CONFIG_DIR=/etc/melco/flickrfs
AUTH_PATH=/etc/melco/flickrfs/auth.xml
FLICKRFS_CACHE=/etc/init.d/flickrfs_cache.sh
FLICKRFS_CACHE_LOCK_FILE=/var/run/flickrfs_cache.pid
FLICKRFS_WATCH=/etc/init.d/flickrfs_watch.sh
FLICKRFS_WATCH_LOCK_FILE=/var/run/flickrfs_watch.pid
#FLICKRFS_INOTIFY_LOCK_FILE=/var/run/flickrfs_inotify.pid
FLICKRFS_FIND_LOCK_FILE=/var/run/flickrfs_find.pid
FLICKRFS_SYNC_LOCK_FILE=/var/run/flickrfs_sync.pid
RSYNC_LOCK_FILE=/var/run/flickrfs_rsync.pid
CONNECT_LOCK_FILE=/var/run/flickrfs_connect.pid

SPOOL_DIR=`libbuffalo_bin GetSpoolDir`
WATCH_DIR="${SPOOL_DIR}/flickr"
mnt=$2
mint=$3
TIMEOUT=60
TIMEOUT_RMM=20

if [ -f ${FLICKRFS_CONFIG_DIR}/flickrfs ]; then
	. /etc/melco/flickrfs/flickrfs 
fi

ExitCleanup()
{
	error_code=${1}
	if [ $error_code != 0 ] ; then
		echo "failed to mount"
	fi
	exit ${error_code}
}

SafeReleaseLock()
{
	if [ -f ${CONNECT_LOCK_FILE} ] ; then
		rm ${CONNECT_LOCK_FILE}
	fi
	return 0
}

ConnectionLock()
{
	if [ ! -f ${CONNECT_LOCK_FILE} ] ; then
		echo $$ > ${CONNECT_LOCK_FILE}
	fi
	return 0
}

ProcessCheck()
{
	if [ -s ${FLICKRFS_WATCH_LOCK_FILE} ]; then
		local pid=`cat ${FLICKRFS_WATCH_LOCK_FILE}`
		echo "your find pid is $pid"
		kill -CONT ${pid}
		local isAlive=$?
		if [ $isAlive = 1 ]; then
			rm ${FLICKRFS_WATCH_LOCK_FILE}
		fi
	fi
}

ConnectionStart()
{
	if [ ! -d $mntpoint ]; then
	  local cnt=$TIMEOUT_RMM
	  echo "Waiting until mountpoint will be changed or created."
	  while [ ! -d $mntpoint ];
	  do
	       echo $cnt
	       if [ 0 -gt $cnt ]; then
	        	echo "Request Timeout"
	        	return -6
	       fi
	  	sleep 1
	  	let cnt=${cnt}-1
	  done
	  if [ ! -d $mntpoint ]; then
	  	echo "mountpoint doesn't exists. terminated."
	  	InitializeAll
	  	exit -2
	  fi
	fi

	echo "=== flickrfs.sh ==="	
	local isAlive=`ps | grep ${FLICKRFS_DIR_R} | grep -v grep | wc -l`
	if [ ! $isAlive = 0 ]; then
	  echo "Waiting until flickrfs.py will be stopped."
	  local cnt=$TIMEOUT
	  while [ ! $isAlive = 0 ];
	  do
	        echo $cnt
	        if [ 0 -gt $cnt ]; then
	        	echo "Request Timeout"
	        	return -6
	        fi
	  	sleep 1
	  	let cnt=${cnt}-1
	        isAlive=`ps | grep ${FLICKRFS_DIR_R} | grep -v grep | wc -l`	
	  done
	fi
	
	echo "Starting FlickrFS..."
	if [ ! -f ${FLICKRFS_CONFIG_DIR}/flickrfs ]; then
		WriteConf
	fi
	local isAlive=`ps | grep ${FLICKRFS_DIR_R} | grep -v grep | wc -l`
	if [ ! $isAlive = 0 ]; then
		echo "flickrfs is already running."		
		return -1
	fi

	if [ $mnt ] ; then
		local _point=$mnt
		#if [ ! -d $_poiont/flickr ] ; then
		mkdir $_point/flickr
		#fi
		MOUNT_POINT=$mnt/flickr
		mini_token=$mint
	else
		local _point=${mntpoint}                                                                                                                                                                             
		if [ ! -d $_point/flickr ] ; then
			mkdir $_point/flickr
		fi
		MOUNT_POINT=${mntpoint}/flickr
	fi
	if [ -d $MOUNT_POINT/stream ] ; then
		rmdir $MOUNT_POINT/stream
	fi
	#cat /etc/melco/flickrfs/flickrfs >/dev/console 2>&1
	export HOME='/root' && nice $PRIORITY ${FLICKRFS_DIR}/flickrfs.py $MOUNT_POINT ${mini_token}
	if [ ! -d $MOUNT_POINT/stream ] ; then 
		mkdir $MOUNT_POINT/stream
	fi
	local isAlive=`ps | grep ${FLICKRFS_DIR_R} | grep -v grep | wc -l`
	if [ $isAlive = 0 ]; then
                sed -e "s/^flickrfs_service=\([^.]*\)/flickrfs_service=off/g" ${FLICKRFS_CONFIG_DIR}/flickrfs | cat > ${FLICKRFS_CONFIG_DIR}/_flickrfs
                mv ${FLICKRFS_CONFIG_DIR}/_flickrfs ${FLICKRFS_CONFIG_DIR}/flickrfs
		return -1
		#ReconnectChallenge $MOUNT_POINT $mini_token
	else
		auth_file=$AUTH_PATH
		if [ -e $AUTH_PATH ]; then
			local username=`grep username ${auth_file} | sed -e "s/.*username=\"\([^.]*\)\" fullname=\".*\" \/>/\1/g"`
			sed -e "s/^username=\([^.]*\)/username=$username/g" ${FLICKRFS_CONFIG_DIR}/flickrfs | cat > ${FLICKRFS_CONFIG_DIR}/_flickrfs
			mv ${FLICKRFS_CONFIG_DIR}/_flickrfs ${FLICKRFS_CONFIG_DIR}/flickrfs
		fi
	fi

	return 0
}

ReconnectChallenge()
{
	local isAlive=`ps | grep ${FLICKRFS_DIR_R} | grep -v grep | wc -l`  
	if [ ! $isAlive = 0 ]; then
		echo "flickrfs is now running."
		return 0
	else
		echo "flickrfs reconnectchallenge"
		while [ $isAlive = 0 ]
		do
			${FLICKRFS_DIR}/flickrfs.py $MOUNT_POINT ${mini_token}
			isAlive=`ps | grep ${FLICKRFS_DIR_R} | grep -v grep | wc -l`
		done
	fi  
	return 0
}

CreateFlickrDir()
{
	return 0
}

ConnectionStop()
{
	echo "=== flickrfs.sh ==="
	if [ -f ${FLICKRFS_CACHE_LOCK_FILE} ] ; then
		local pid=`cat ${FLICKRFS_CACHE_LOCK_FILE}`
		#echo "pid is"
		#echo $pid
		local isAlive=`ps w | grep $pid | grep -v grep | wc -l`
		if [ ! $isAlive = 0 ]; then
			kill -9 ${pid}
		fi
		rm ${FLICKRFS_CACHE_LOCK_FILE}
	fi
	
	if [ -f ${FLICKRFS_WATCH_LOCK_FILE} ] ; then
		local pid=`cat ${FLICKRFS_WATCH_LOCK_FILE}`
	        #echo "pid is"
	        #echo $pid
		#local isAlive=`ps w | grep $pid | grep -v grep | wc -l`
		#if [ ! $isAlive = 0 ]; then
		#	killall -9 flickrfs_watch.sh
		#fi
		#rm ${FLICKRFS_WATCH_LOCK_FILE}
		local for_upload=`find $WATCH_DIR -type f -maxdepth 3 \( -name \*.jpg_ -o -name \*.JPG_ -o -name \*.jpeg_ -o -name \*.JPEG_ -o -name \*.gif_ -o -name \*.GIF_ -o -name \*.tif_ -o -name \*.TIF_ -o -name \*.tiff_ -o -name \*.TIFF_ -o -name \*.bmp_ -o -name \*.BMP_ \) ! -name "*.meta" -print | wc -l`
		if [ -d ${WATCH_DIR} ]; then
			if [ ${for_upload} -eq 0 ]; then
				rm -rf $WATCH_DIR
			fi
		fi
		sleep 3
                local isAlive=`ps w | grep $pid | grep -v grep | wc -l`
		if [ ! $isAlive = 0 ]; then                            
			killall -9 flickrfs_watch.sh                   
		fi
		rm ${FLICKRFS_WATCH_LOCK_FILE}  
	fi
	
        if [ -f ${RSYNC_LOCK_FILE} ] ; then
		local pid=`cat ${RSYNC_LOCK_FILE}`
                local isAlive=`ps w | grep $pid | grep -v grep | wc -l`
		if [ ! $isAlive = 0 ]; then                            
			kill -9 ${pid}                                 
		fi
		rm ${RSYNC_LOCK_FILE}
	fi 

        if [ -f ${FLICKRFS_SYNC_LOCK_FILE} ] ; then                
		local pid=`cat ${FLICKRFS_SYNC_LOCK_FILE}`
                local isAlive=`ps w | grep $pid | grep -v grep | wc -l`                                                                                                                                     
		if [ ! $isAlive = 0 ]; then                                                                                                                                                                 
			kill -9 ${pid} 
		fi
		rm ${FLICKRFS_SYNC_LOCK_FILE}
	fi 

	if [ -f ${FLICKRFS_FIND_LOCK_FILE} ] ; then
		local pid=`cat ${FLICKRFS_FIND_LOCK_FILE}`
		local isAlive=`ps w | grep $pid | grep -v grep | wc -l`
		if [ ! $isAlive = 0 ]; then
			kill -9 ${pid}
		fi
		rm ${FLICKRFS_FIND_LOCK_FILE}
	fi

	if [ ! $2 ] ; then
		echo "force stopping mode"
		MOUNT_POINT=${mntpoint}/flickr
	else
		MOUNT_POINT=$2/flickr
		echo "Stopping FlickrFS..."
	fi
	local isAlive=`ps | grep ${FLICKRFS_DIR_R} | grep -v grep | wc -l`
	if [ $isAlive = 0 ]; then
		echo "flickrfs is already unmounted"
		return -2
	else
                ${FUSERMOUNT_DIR}/fusermount -uz $MOUNT_POINT
                local isAlive=`ps | grep ${FLICKRFS_DIR_R} | grep -v grep | wc -l`
                if [ $isAlive = 0 ]; then
                	echo "flickrfs is unmounted"
                else
			echo "flickrfs is busy, force unmount mode"
			${FUSERMOUNT_DIR}/fusermount -uz $MOUNT_POINT
			#pid=`cat /var/run/xl2tpd.pid`
			#kill -KILL $pid
			killall flickrfs.py
			#sleep 7
			if [ -d $MOUNT_POINT ]; then
				rm -rf $MOUNT_POINT
			fi
			return -3
               fi
	fi
	local isAlive=`ps | grep flickrfs.sh | grep -v grep | wc -l`
	if [ $isAlive != 0 ] ; then
		echo "stopping remaining process..."
		${FUSERMOUNT_DIR}/fusermount -uz $MOUNT_POINT
		killall flickrfs.py
		#killall ${FLICKRFS_DIR_R}
	fi
	#sleep 5
	if [ -d $MOUNT_POINT ]; then
		rm -rf $MOUNT_POINT
	fi
	return 0
}

WriteConf()
{
	cp ${FLICKRFS_DIR_R}/flickrinit ${FLICKRFS_CONFIG_DIR}/flickrfs
	return 0
}

ClearConf()
{
	return 0
}

CheckConf()
{
	return 0
}

BootChoice()
{
	if [ ! -f ${FLICKRFS_CONFIG_DIR}/flickrfs ]; then
		echo "configfile doesn't exists. terminated."
		exit -1
	fi

	if [ "${flickrfs_service}" = "on" ] ; then
		ConnectionStart
	else
		ConnectionStop
	fi
	return 0
}

SyncPic()
{

	return 0
}

ShowStatus()
{
	local isAlive=`ps | grep ${FLICKRFS_DIR_R} | grep -v grep | wc -l`
	if [ $isAlive = 0 ]; then
		echo "flickrfs is now stopping..."
	else
		echo "flickrfs is now running..."
	fi
	return 0
}

InitializeAuthentication()
{
	local auth_file=$1
	echo "Initializing FlickrFS..."
	sed -e "s/^flickrfs_service=\([^.]*\)/flickrfs_service=off/g" ${FLICKRFS_CONFIG_DIR}/flickrfs | cat > ${FLICKRFS_CONFIG_DIR}/_flickrfs
	mv ${FLICKRFS_CONFIG_DIR}/_flickrfs ${FLICKRFS_CONFIG_DIR}/flickrfs
	rm $auth_file

	ConnectionStop 
}

InitializeAll()
{
	#local auth_file=$AUTH_PATH
	echo "Initializing FlickrFS..."
	#sed -e "s/^flickrfs_service=\([^.]*\)/flickrfs_service=off/g" ${FLICKRFS_CONFIG_DIR}/flickrfs | cat > ${FLICKRFS_CONFIG_DIR}/_flickrfs
	#mv ${FLICKRFS_CONFIG_DIR}/_flickrfs ${FLICKRFS_CONFIG_DIR}/flickrfs
	ConnectionStop	

	rm $AUTH_PATH
	if [ -f ${FLICKRFS_CONFIG_DIR}/flickrfs ]; then
		rm -f ${FLICKRFS_CONFIG_DIR}/flickrfs
	fi
	if [ ! -d ${FLICKRFS_CONFIG_DIR} ]; then
		mkdir ${FLICKRFS_CONFIG_DIR}
	fi
	cp $FLICKRFS_DIR_R/flickrinit ${FLICKRFS_CONFIG_DIR}/flickrfs
}

ShowUsername()
{
	local auth_file=$1
	if [ -n "${auth_file}" ] ; then
		grep username ${auth_file} | sed -e "s/.*username=\"\([^.]*\)\" fullname=\".*\" \/>/\1/g"
	else                                                                                   
		echo "path to auth.xml required."                                                      
	fi
}

ret=0
command=$1
ConnectionLock

case $command in
real_start)
	ConnectionStart
	;;
stop)
	ConnectionStop
	;;
restart)
	ConnectionStop
	sleep 5
	ConnectionStart
	;;
sync)
	SyncPic $2
	;;
status)
	ShowStatus
	;;
username)
	ShowUsername $2
	;;
init)
	InitializeAuthentication $2 $3
	;;
init_all)
	InitializeAll
	;;
start)
	BootChoice
	ret=$?
	;;
write_conf)
	WriteConf
	ret=$?
	;;
clear_conf)
	ClearConf
	ret=$?
	;;
check_conf)
	CheckConf
	ret=$?
	;;
ping_test)
	PingTest
	ret=$?
	;;
nslookup_test)
	NslookupTest
	ret=$?
	;;
dead_or_alive)
	ProcessCheck
	ret=$?
	;;
*)
	echo "Usage: flickrfs.sh [option] [path]

List directory contents

	Options:
	start     start flickrfs and initialize mount point
	stop      stop flickrfs
	status    show status
	init	   initialize authentification
	username  get username from auth.xml"
	;;
esac

SafeReleaseLock
ExitCleanup ${ret}

