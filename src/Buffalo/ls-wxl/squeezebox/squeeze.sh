#!/bin/bash
LOCKFILE="/var/lock/squeeze"
if ! ln -s $LOCKFILE $LOCKFILE ; then
	exit 1
fi

trap "rm $LOCKFILE" EXIT

BASEDIR="/usr/local/squeezebox"
PERL="/usr/local/squeezebox/perl-5.10.0/bin/perl"
PIDFILE="/var/run/squeezebox.pid"
MYSQLPIDFILE="/usr/local/squeezebox/mysqlpid"
INITFILES_TGZ="/root/.files/initfile.tar.gz"
SERVER_PREFS="/usr/local/squeezebox/prefs/server.prefs"
PREFS_DIR="/usr/local/squeezebox/prefs"
SBSERVICE="/etc/melco/squeezeboxserver/squeezeboxserver_service"
MELCO_DIR="/etc/melco/squeezeboxserver"
#SCHECKFILE="/var/run/squeezecheck"
HTTPPORT=""
AUDIO_DIR=""
SETTING_READ=0
NAS_FEATURE="/etc/nas_feature"
BUSYBOX_KILL="/bin/busybox kill"
BUSYBOX_PS="/bin/busybox ps"

start()
{
	. $NAS_FEATURE
	if [[ $SUPPORT_SQUEEZEBOX -ne 1 ]]; then
		echo "Sqeezebox Server is not supported." >&2
		return 1
	fi

	if [[ -f $PIDFILE ]]; then
		echo "Squeezebox Server is already running." >&2
		return 1
	fi
	
	if [[ -f ${BASEDIR}/squeezeboxserver_service ]]; then
		#first time
		chown -R root:root ${BASEDIR}		
                chmod -R 777 ${BASEDIR}
		chown -R root:root ${MELCO_DIR}		
                chmod -R 777 ${MELCO_DIR}
		rm ${BASEDIR}/squeezeboxserver_service
		ln -s /dev/null ${BASEDIR}/dummylog.err
        fi

	#if [[ ! -f /var/log/squeezebox/server.log ]]; then 
		touch /var/log/squeezebox/server.log
		touch /var/log/squeezebox/perfmon.log
		touch /var/log/squeezebox/scanner.log
		chmod -R 777 /var/log/squeezebox
	#fi

	cp -Rp ${MELCO_DIR}/prefs ${BASEDIR}

	readsetting

	local userinfo=$(id squeezeboxserver 2> /dev/null)
	if [[ -z $userinfo ]]; then
		echo "it seems that squeezeboxserver does'nt exist!" >&2
		useradd -rU -d  /dev/null squeezeboxserver
	fi	

	checkDB

	if [[ $? != 0 ]]; then
		clearCache
	fi

	startsqueeze
	local ret=$?
	#touch $SCHECKFILE
	#startcheck &
	return $ret 
}

readmysqlpid()
{
	local mypidfile=$(getCacheDir)"/squeezebox-mysql.pid"
	while [[ ! -f $mypidfile ]]
	do
		#echo "waiting mysqld..." >&2
		sleep 1
	done

	#echo $(cat $mypidfile) > $MYSQLPIDFILE
	ln -s $mypidfile $MYSQLPIDFILE	
}

startsqueeze()
{
	local cachedir=$(getCacheDir)

	LANG=en_US.UTF-8 ${PERL} ${BASEDIR}/slimserver.pl  --daemon --pidfile ${PIDFILE} --httpport ${HTTPPORT} --cachedir ${cachedir} --failsafe --logdir /var/log/squeezebox/
	local ret=$?
	readmysqlpid
	return $ret
}

startcheck()
{
	sleep 10 
	local pid
	local tmp
	local mysqlpid
	pid=$(cat ${PIDFILE})
	tmp=$(${BUSYBOX_PS} | grep "^[ ]*${pid} squeezebox")
	#echo "tmp=${tmp}"
	if [[ -z $tmp ]]; then
		echo "DB is incorrect?" >&2
		#clear cache and try again
		mysqlpid=$(cat $MYSQLPIDFILE)
		${BUSYBOX_KILL} ${mysqlpid}
		clearCache
		startsqueeze
	fi
	rm $SCHECKFILE
}

checkDB()
{
	local dbdir
	local ret=0
	local file
	local dbname
	dbdir=$(getCacheDir)"/MySQL/slimserver"

	for file in $(find ${dbdir} -name "*.MYI")
	do
		echo "checking ${file}" >&2
		myisamchk -ss ${file}
		ret=$?
		if [[ ${ret} != 0 ]]; then
			dbname=$(echo ${file} | sed -e 's/\.[^.]*$//')
			myisamchk -o ${dbname}
			ret=$?
			if [[ ${ret} != 0 ]]; then
				echo "can't fix!" >&2
				return ${ret} 
			else
				echo "fixed" >&2
			fi
		fi
	done

	return ${ret}
}

clearCache()
{
	local cachedir=$(getCacheDir)
	rm -r ${cachedir}/*
}

readsetting()
{
	local array1
	local disk1
	local disk2
	local defaultdir=""
	local service
	local httpport
	local audiodir=""
	local playlistdir=""
	local audiodirchange=0
	local playlistdirchange=0

	if [[ $SETTING_READ -ne 0 ]]; then
		return
	fi
	SETTING_READ=1
	
	. $SBSERVICE

	if [[ "$service" != "on" ]]; then
		echo "Squeezebox Server is disabled." >&2
		exit 1
	fi

	. /etc/melco/diskinfo

	if [[ "${array1}" = "off" ]] || [[ "${array1}" = "" ]]; then
		if [[ ! -d /mnt/disk1/share ]] && [[ "${disk1}" != "normal" ]] && [[ "${disk2}" = "normal" ]] && [[ -d /mnt/disk2/share ]]; then
			defaultdir="/mnt/disk2/share"
		else
			defaultdir="/mnt/disk1/share"
		fi
	else
        	defaultdir="/mnt/array1/share"
	fi


	HTTPPORT=$httpport
	writePortList	

	if [[ -z $audiodir ]] ; then
	      audiodir=$defaultdir
	      audiodirchange=1
	fi

	if [[ -z $playlistdir ]] ; then
		playlistdir=$audiodir
		playlistdirchange=1	
	fi

	if [[ ! -d $audiodir ]] || [[ ! -d $playlistdir ]] ; then
		echo "Data directory does not exist." >&2
		exit 1
	fi

	#save audiodir to make spool dir
	AUDIO_DIR=$audiodir

	audiodir=$(echo "$audiodir" | sed -e 's/\//\\\//g')
	playlistdir=$(echo "$playlistdir" | sed -e 's/\//\\\//g')

	if [[ $audiodirchange -eq 1 ]]; then
		sed -i -e "s/audiodir=.*/audiodir=\"$audiodir\"/" $SBSERVICE
	fi

	if [[ $playlistdirchange -eq 1 ]]; then
		sed -i -e "s/playlistdir=.*/playlistdir=\"$playlistdir\"/" $SBSERVICE
	fi
	
	sed -i -e "s/^audiodir: .*/audiodir: $audiodir/" -e "s/^playlistdir: .*/playlistdir: $playlistdir/" $SERVER_PREFS

	#artfolder must be subdirectory of audiodir
	fixartfolder
}

fixartfolder()
{
	local artfolder=$(cat $SERVER_PREFS | grep '^artfolder:' | sed -e 's/artfolder: //')
	local audiodir=$(cat $SERVER_PREFS | grep '^audiodir:' | sed -e 's/audiodir: //')
	local issub
	
	if [[ "$artfolder" != "''" ]]; then
		issub=$(echo $artfolder | grep "^$audiodir")
		if [[ "$issub" = "" ]]; then
			audiodir=$(echo "$audiodir" | sed -e 's/\//\\\//g')
			sed -i -e "s/^artfolder: .*/artfolder: $audiodir/" $SERVER_PREFS
		fi
	fi
}

writePortList() 
{
	local file="/etc/melco/portlist.d/squeezebox"
	local SqueezeboxWebUI

	. $file

	if [[ "$SqueezeboxWebUI" != "$HTTPPORT" ]]; then
		sed -i -e "s/SqueezeboxWebUI=.*/SqueezeboxWebUI=$HTTPPORT/" $file
	fi
}

getSpoolDir()
{
	local spool;
	spool=$(libbuffalo_bin GetSpoolDir)

	if [[ -z $spool ]]; then
		if [[ -z $AUDIO_DIR ]]; then
			readsetting
		fi
		spool=$(echo $AUDIO_DIR | sed -e "s/\(\/mnt\/\(disk\|array\)[^\/]*\/\).*/\1spool/")
		mkdir $spool
		chmod 755 $spool
	fi

	echo $spool
}

getCacheDir()
{
	local cache=$(getSpoolDir)/slim-data

	if [[ ! -d $cache ]]; then
		mkdir $cache
		chmod 777 $cache
	fi

	echo $cache
}

stop()
{
	#local tcount=0
	#local tout=15
	echo "Stopping Squeezebox Server..."

	#while [[ -f $SCHECKFILE ]]
	#do
	#	sleep 1
	#	echo "wait check..."
	#	tcount=$((tcount + 1))
	#	if [[ $tcount -ge $tout ]]; then
	#		echo "wait check timeout" >&2
	#		rm $SCHECKFILE
	#		break;
	#	fi
	#done

	#chmod 777 "/usr/local/squeezebox/prefs/tracks_persistent.json"
	chmod -R 777 ${PREFS_DIR}
	if [[ -f ${PIDFILE} ]]; then
		local slimpid=$(cat ${PIDFILE})
		local mysqlpid=$(cat ${MYSQLPIDFILE})
		${BUSYBOX_KILL} ${slimpid}
		if [[ $? -ne 0 ]]; then
			echo "Squeezebox Server is not runnnig?" >&2
			return 1
		fi
		deadWait $slimpid $mysqlpid
		rm ${PIDFILE}
		rm ${MYSQLPIDFILE}
		echo "Squeezebox Server stopped." >&2
	else
		echo "Squeezebox Server is not running!" >&2
		return 1
	fi
	cp -Rp $PREFS_DIR $MELCO_DIR 
	return 0
}

deadWait()
{
	local slimpid=$1
	local mysqlpid=$2
	local tcount=0
	local tout=10
	while ${BUSYBOX_PS} | grep -q "^[ ]*${slimpid} squeezebox\|^[ ]*${mysqlpid} squeezebox"
	do
		tcount=$(expr $tcount + 1)
		if [[ $tcount -ge $tout ]]; then
			echo "kill timeout" >&2
			return
		fi
		echo "waiting..."
		sleep 2
	done
}

init()
{
	local prefsdir=${MELCO_DIR}/prefs
	local config_files="${prefsdir#/} ${SBSERVICE#/}"
	#echo ${config_files}
	tar zxf ${INITFILES_TGZ} -C / $config_files
}

case "$1" in
  start)
    start
    ;;
  stop)
    stop
    ;;
  restart)
    stop
    start
    ;;
  init)
    stop
    init
    start
    ;;
  refresh_start)
     stop
     clearCache
     start
     ;;
esac
