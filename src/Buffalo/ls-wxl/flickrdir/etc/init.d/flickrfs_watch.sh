#!/bin/sh
. /etc/melco/flickrfs/flickrfs

PRIORITY=-0
INOTIFYWAIT=/usr/bin/inotifywait
SPOOL_DIR=`libbuffalo_bin GetSpoolDir`
WATCH_DIR="${SPOOL_DIR}/flickr"
PIDFILE=/var/run/flickrfs_watch.pid
PROCESS_CHECK="/etc/init.d/flickrfs.sh dead_or_alive"
INOTIFY_LOCK_FILE=/var/run/flickrfs_inotify.pid
MOUNT_POINT=${mntpoint}/flickr
FAIL_TO_UPLOAD=${mntpoint}/fail_to_upload
RETRY=2

PIDFileCheck()
{
	$PROCESS_CHECK
	if [ -f $PIDFILE ]; then
		echo "$0 already running." >&2
		return 1
	fi

	if [ -f $INOTIFY_LOCK_FILE ]; then
		local INOTIFY_PID=`cat ${INOTIFY_LOCK_FILE}`
	fi

	echo $$ > $PIDFILE

	trap "echo trapped; rm $PIDFILE; rm -rf $WATCH_DIR; exit 1" 1 2 3 15
}

preprocess()
{
	sleep 30
        if [ ! -d ${WATCH_DIR} ]; then
		mkdir -p ${WATCH_DIR}
	fi
	nice $PRIORITY find $WATCH_DIR -type f -maxdepth 3 \( -name \*.jpg_ -o -name \*.JPG_ -o -name \*.jpeg_ -o -name \*.JPEG_ -o -name \*.gif_ -o -name \*.GIF_ -o -name \*.tif_ -o -name \*.TIF_ -o -name \*.tiff_ -o -name \*.TIFF_ -o -name \*.bmp_ -o -name \*.BMP_ \) ! -name "*.meta" -print -exec /bin/sh -c 'TARGET=`echo "{}" | sed -e s:^"$WATCH_DIR"::` && cp {} > ${TARGET}' \; > /dev/null 2>&1
}

process()
{
	local status=0
	status=0

#	echo $$ > $INOTIFY_LOCK_FILE

        #echo "$INOTIFYWAIT -m -e close_write $WATCH_DIR | while read path event file"
	nice $PRIORITY $INOTIFYWAIT -r -m -e close_write $WATCH_DIR | while read path event file
	do
		#[ "$file" != "fool" ] && continue
		case "$event" in
		    CLOSE_WRITE,CLOSE)
			case "$file" in
			    *\.jpg_|*\.JPG_|*\.jpeg_|*\.JPEG_|*\.gif_|*\.GIF_|*\.png_|*\.PNG_|*\.tif_|*\.TIF_|*\.tiff_|*\.TIFF_|*\.bmp_|*\.BMP_)
			        #status=$(awk -F= '/Number_of_sessions_currently_running=/ {print $2}' $WATCH_DIR/fool)
			        echo "image file created or modified"
				echo $1 $path$file >> /var/log/flickrfs_upload.log
				if [ "$path" == "\/.\/.\/.\/.\/.\/.webaxs\/thumbnail\/" ]; then
					echo "The image file is for thumbnail, skipped."
					continue
				fi
				TARGET_DIR=`echo "$path" | sed -e s:^"$WATCH_DIR"::`
	                        #SPOOL_FILE=`echo "$path" | sed -e s:_$::`
	                        SPOOL_FILE=`echo "$file" | sed -e s:_$::`
	                        #SPOOL_FILE=${TARGET_DIR}${S_FILE}
				TARGET_FILE=${MOUNT_POINT}${TARGET_DIR}${file}
				echo "${TARGET_FILE}"
				#webaxs thumnail file found, ignored.
				
				#copy to fuse mounted dir
				cp ${path}${file} ${TARGET_FILE}
				result=$?
				echo ${result}
				local cnt=1
		                if [ $result -eq 0 ]; then
		                	rm ${path}${file}
		                fi
		                #fail to upload
				while [ $result -ne 0 ]
				do
                                	if [ $cnt -lt $RETRY ]; then                                   
                                        	echo "Upload Failed"                     
                                        	if [ ! -d ${mntpoint}/fail_to_upload ]; then
                                        		mkdir ${mntpoint}/fail_to_upload
                                        		chown nobody:nogroup ${mntpoint}/fail_to_upload
                                        		chmod 777 ${mntpoint}/fail_to_upload
						fi
                                        	cp ${path}${file} ${mntpoint}/fail_to_upload/${SPOOL_FILE}
                                        	result=$?
                                        	if [ $result -eq 0 ]; then
                                        		rm ${path}{file}
                                        	else
                                        		echo "fatal. maybe sharefolder is broken."
                                        	fi
                                        	break
                                  	fi 
					let cnt=${cnt}+1
					cp ${path}${file} ${TARGET_FILE}
					result=$?
					echo ${result}
					if [ $result -eq 0 ]; then
						rm ${path}${file}
					        break
					fi
				done
				;;  
			    *)  
			  	echo "file found but not for upload file"
			        ;;
			esac
			;;
		    DELETE)
			echo "deleted"
			;;
		    *)
		        echo "other"
		esac
	done
	
	return 0
}

PIDFileCheck || exit 1
[ -x $INOTIFYWAIT ] || exit 1
[ -d $WATCH_DIR ] || mkdir $WATCH_DIR

preprocess
process $1
