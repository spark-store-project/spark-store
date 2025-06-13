#!/bin/bash
export LANGUAGE=en_US
export DEBIAN_FRONTEND=noninteractive
case $1 in 
	ssupdate)
if [ "$(id -u)" != "0" ] ; then
	pkexec "$0" "$@"
	exit
fi
		aptss ssupdate 2>&1 | tee /tmp/spark-store-app-ssupdate-log.txt
		IS_SSUPDATE_ERROR=`cat /tmp/spark-store-app-ssupdate-log.txt | grep "E: "`
		echo "$IS_SSUPDATE_ERROR" > /tmp/spark-store-app-ssupdate-status.txt
		chmod 777 /tmp/spark-store-app-ssupdate-status.txt
		chmod 777 /tmp/spark-store-app-ssupdate-log.txt
	;;

	upgradable-list)
		output=$(env LANGUAGE=en_US /usr/bin/apt -c /opt/durapps/spark-store/bin/apt-fast-conf/aptss-apt.conf list --upgradable -o Dir::Etc::sourcelist="/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/sparkstore.list" -o Dir::Etc::sourceparts="/dev/null" -o APT::Get::List-Cleanup="0" | awk NR\>1)

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
if [ "$(id -u)" != "0" ] ; then
	pkexec "$0" "$@"
	exit
fi

		aptss install "${@:2}" --only-upgrade  2>&1 | tee /tmp/spark-store-app-upgrade-log.txt
        sed -i '1i--------------------------------------------------------------' /tmp/spark-store-app-upgrade-log.txt
        sed -i '1i更新失败可能是由于系统版本过低，您可先【卸载】此应用后再在商店【安装】此应用来尝试修复此问题，商店会在安装时尝试自动解决问题。若仍无法解决，请按照指引进行反馈' /tmp/spark-store-app-upgrade-log.txt
		chmod 777 /tmp/spark-store-app-upgrade-log.txt
		IS_UPGRADE_ERROR=`cat /tmp/spark-store-app-upgrade-log.txt | grep "Package manager quit with exit code."`
		echo "$IS_UPGRADE_ERROR" > /tmp/spark-store-app-upgrade-status.txt
	;;
	test-install-app)
if [ "$(id -u)" != "0" ] ; then
	pkexec "$0" "$@"
	exit
fi

try_run_output=$(aptss --dry-run install $2)
try_run_ret="$?"

if [ "$try_run_ret" -ne 0 ]
  then
    echo "Package manager quit with exit code.Here is the log" 
    echo "包管理器以错误代码退出.日志如下" 
    echo
    echo -e "${try_run_output}"
    echo "Will try after run aptss update"
    echo "将会在aptss update之后再次尝试"
    aptss update
    echo ----------------------------------------------------------------------------
	try_run_output=$(aptss --dry-run install $2)
	try_run_ret="$?"
  		if [ "$try_run_ret" -ne 0 ]
  		then
  		  echo "Package manager quit with exit code.Here is the log" 
   		 echo "包管理器以错误代码退出.日志如下" 
   		 echo
    		echo -e "${try_run_output}"
    		exit "$try_run_ret"
 		 fi

fi
	exit 0
	;;
	
	clean-log)

	rm -f /tmp/spark-store-app-ssupdate-status.txt /tmp/spark-store-app-ssupdate-log.txt /tmp/spark-store-app-upgrade-log.txt /tmp/spark-store-app-upgrade-status.txt
	;;
esac
