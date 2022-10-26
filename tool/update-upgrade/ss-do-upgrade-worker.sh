#!/bin/bash
if [ "$(id -u)" != "0" ];then
pkexec "$0" "$@"
exit
fi


case $1 in 
	ssupdate)
	aptss ssupdate
	;;
	upgradable-list)
	bwrap --dev-bind / / --bind '/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/sparkstore.list' /etc/apt/sources.list.d/sparkstore.list apt list --upgradable -o Dir::Etc::sourcelist="sources.list.d/sparkstore.list"     -o Dir::Etc::sourceparts="-" -o APT::Get::List-Cleanup="0" | awk 'BEGIN {FS="/"} {print $1}' | awk NR\>1
	;;
	upgrade-app)
	aptss install "${@:2}" --only-upgrade
	
esac