#!/bin/bash
if [ "$(id -u)" != "0" ] ; then
	pkexec "$0" "$@"
	exit
fi

case $1 in 
	ssupdate)
		aptss ssupdate 2>&1 | tee /tmp/spark-store-app-ssupdate-log.txt
		IS_SSUPDATE_ERROR=`cat /tmp/spark-store-app-ssupdate-log.txt | grep "E: "`
		echo "$IS_SSUPDATE_ERROR" > /tmp/spark-store-app-ssupdate-status.txt
	;;

	upgradable-list)
		output=$(env LANGUAGE=en_US bwrap --dev-bind / / --bind '/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/sparkstore.list' /etc/apt/sources.list.d/sparkstore.list apt list --upgradable -o Dir::Etc::sourcelist="sources.list.d/sparkstore.list" -o Dir::Etc::sourceparts="/dev/null" -o APT::Get::List-Cleanup="0" | awk NR\>1)

		IFS_OLD="$IFS"
		IFS=$'\n'

		for line in $output ; do
			PKG_NAME=$(echo $line | awk -F '/' '{print $1}')
			PKG_NEW_VER=$(echo $line | awk -F ' ' '{print $2}')
			PKG_CUR_VER=$(echo $line | awk -F ' ' '{print $6}' | awk -F ']' '{print $1}')
			echo "${PKG_NAME} ${PKG_NEW_VER} ${PKG_CUR_VER}"
		done

		IFS="$IFS_OLD"
	;;

	upgrade-app)
		aptss install "${@:2}" --only-upgrade  2>&1 | tee /tmp/spark-store-app-upgrade-log.txt
		IS_UPGRADE_ERROR=`cat /tmp/spark-store-app-upgrade-log.txt | grep "Package manager quit with exit code."`
		echo "$IS_UPGRADE_ERROR" > /tmp/spark-store-app-upgrade-status.txt
	;;
	
	clean-log)
		rm -f /tmp/spark-store-app-ssupdate-status.txt /tmp/spark-store-app-ssupdate-log.txt /tmp/spark-store-app-upgrade-log.txt /tmp/spark-store-app-upgrade-status.txt
	;;
esac
