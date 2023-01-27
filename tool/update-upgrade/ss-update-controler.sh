#!/bin/bash

##load transhell
function load_transhell()
{
local WORK_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")"  && pwd)"
local CURRENT_LANG="$(echo ${LANG%.*})"
if [ -e "/usr/share/$(basename $0)/transhell/$(basename $0)_en_US.transhell" ]; then source /usr/share/$(basename $0)/transhell/$(basename $0)_en_US.transhell; echo "Loading transhell from /usr/share/$(basename $0)/transhell/$(basename $0)_en_US.transhell ..."; fi
if [ -e "/usr/share/$(basename $0)/transhell/$(basename $0)_$CURRENT_LANG.transhell" ]; then source /usr/share/$(basename $0)/transhell/$(basename $0)_$CURRENT_LANG.transhell; echo "Loading transhell from /usr/share/$(basename $0)/transhell/$(basename $0)_$CURRENT_LANG.transhell ..."; fi
if [ -e "${WORK_PATH}/transhell/$(basename $0)_en_US.transhell" ]; then source ${WORK_PATH}/transhell/$(basename $0)_en_US.transhell; echo "Loading transhell from ${WORK_PATH}/transhell/$(basename $0)_en_US.transhell ..."; fi
if [ -e "${WORK_PATH}/transhell/$(basename $0)_$CURRENT_LANG.transhell" ]; then source ${WORK_PATH}/transhell/$(basename $0)_$CURRENT_LANG.transhell; echo "Loading transhell from ${WORK_PATH}/transhell/$(basename $0)_$CURRENT_LANG.transhell ..."; fi

echo "-----------------------------------------------------------------------------"
}

function update_transhell()
{
local WORK_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")"  && pwd)"
local CURRENT_LANG="$(echo ${LANG%.*})"
if [ -e "/usr/share/$(basename $0)/transhell/$(basename $0)_en_US.transhell" ]; then source /usr/share/$(basename $0)/transhell/$(basename $0)_en_US.transhell; fi
if [ -e "/usr/share/$(basename $0)/transhell/$(basename $0)_$CURRENT_LANG.transhell" ]; then source /usr/share/$(basename $0)/transhell/$(basename $0)_$CURRENT_LANG.transhell; fi
if [ -e "${WORK_PATH}/transhell/$(basename $0)_en_US.transhell" ]; then source ${WORK_PATH}/transhell/$(basename $0)_en_US.transhell; fi
if [ -e "${WORK_PATH}/transhell/$(basename $0)_$CURRENT_LANG.transhell" ]; then source ${WORK_PATH}/transhell/$(basename $0)_$CURRENT_LANG.transhell; fi

}

load_transhell

endloop=0

#####################检测是否启动过了更新检测工具
while [ $endloop -eq 0 ] ;do

if [  -e /etc/systemd/system/multi-user.target.wants/spark-update-notifier.service ];then 
text_update_open="${TRANSHELL_CONTENT_CLOSE}"
#已经开启了就显示关闭
else
text_update_open="${TRANSHELL_CONTENT_OPEN}"
fi

update_transhell

option=`zenity --list --text="${TRANSHELL_CONTENT_WELCOME_AND_CHOOSE_ONE_TO_RUN}" --column 数字 --column=${TRANSHELL_CONTENT_OPTIONS}  --print-column=2  --height 350 --width 760  0 "${TRANSHELL_CONTENT_OPEN_OR_CLOSE_UPGRADE_CHECK}" 1 "${TRANSHELL_CONTENT_CHECK_FOR_UPDATE}"  2 "${TRANSHELL_CONTENT_EXIT}" --hide-column=1 --print-column=1` 

case $option in 
	0)
	if [  -e /etc/systemd/system/multi-user.target.wants/spark-update-notifier.service ];then 
	zenity --info --icon-name=spark-store --height 150 --width 200 --text "${TRANSHELL_CONTENT_CLOSING_UPGRADE_CHECK}" --timeout=2 
	pkexec systemctl disable spark-update-notifier
	zenity --info --icon-name=spark-store --height 150 --width 200 --text "${TRANSHELL_CONTENT_CLOSED}" --timeout=2
	else
	zenity --info --icon-name=spark-store --height 150 --width 200 --text  "${TRANSHELL_CONTENT_OPENING_UPGRADE_CHECK}" --timeout=2 
	pkexec systemctl enable spark-update-notifier
	pkexec service spark-update-notifier start
	zenity --info --icon-name=spark-store --height 150 --width 200 --text "${TRANSHELL_CONTENT_OPENED}" --timeout=2
	fi
	;;

	1)
	/opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade.sh
	;;

	2)
	exit 0
	;;
	*) 
	exit 0
esac

done
