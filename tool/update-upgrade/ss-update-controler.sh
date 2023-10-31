#!/bin/bash

source /opt/durapps/spark-store/bin/bashimport/transhell.amber
load_transhell_debug

endloop=0

#####################检测是否启动过了更新检测工具
while [ $endloop -eq 0 ] ;do

if [  ! -e $HOME/.config/spark-union/spark-store/ssshell-config-do-not-show-upgrade-notify ];then 
text_update_open="${TRANSHELL_CONTENT_CLOSE}"
#已经开启了就显示关闭
else
text_update_open="${TRANSHELL_CONTENT_OPEN}"
fi

if [  ! -e $HOME/.config/spark-union/spark-store/ssshell-config-do-not-create-desktop ];then 
CONTENT_SET_CREATE_DESKTOP="${TRANSHELL_CONTENT_CLOSE_CREATE_DESKTOP}"
#已经开启了就显示关闭
else
CONTENT_SET_CREATE_DESKTOP="${TRANSHELL_CONTENT_OPEN_CREATE_DESKTOP}"
fi

update_transhell

option=$(zenity --list --text="${TRANSHELL_CONTENT_WELCOME_AND_CHOOSE_ONE_TO_RUN}" --column 数字 --column=${TRANSHELL_CONTENT_OPTIONS}  --print-column=2  --height 350 --width 760  0 "${TRANSHELL_CONTENT_OPEN_OR_CLOSE_UPGRADE_CHECK}" 1 "${CONTENT_SET_CREATE_DESKTOP}" 2 "${TRANSHELL_CONTENT_CHECK_FOR_UPDATE}" 3 "${TRANSHELL_CONTENT_EXIT}" --hide-column=1 --print-column=1)

case $option in 
	0)
	if [  ! -e $HOME/.config/spark-union/spark-store/ssshell-config-do-not-show-upgrade-notify ];then 
	mkdir -p $HOME/.config/spark-union/spark-store/
	touch $HOME/.config/spark-union/spark-store/ssshell-config-do-not-show-upgrade-notify
	zenity --info --icon-name=spark-store --height 150 --width 200 --text "${TRANSHELL_CONTENT_CLOSED}" --timeout=2
	else
	rm -f  $HOME/.config/spark-union/spark-store/ssshell-config-do-not-show-upgrade-notify
	zenity --info --icon-name=spark-store --height 150 --width 200 --text "${TRANSHELL_CONTENT_OPENED}" --timeout=2
	fi
	;;
	1)
	if [  ! -e $HOME/.config/spark-union/spark-store/ssshell-config-do-not-create-desktop ];then 
	mkdir -p $HOME/.config/spark-union/spark-store/
	touch $HOME/.config/spark-union/spark-store/ssshell-config-do-not-create-desktop
	zenity --info --icon-name=spark-store --height 150 --width 200 --text "${TRANSHELL_CONTENT_CLOSED}" --timeout=2
	else
	rm -f  $HOME/.config/spark-union/spark-store/ssshell-config-do-not-create-desktop
	zenity --info --icon-name=spark-store --height 150 --width 200 --text "${TRANSHELL_CONTENT_OPENED}" --timeout=2
	fi
	;;
	2)
	/opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade.sh
	;;

	3)
	exit 0
	;;
	*) 
	exit 0
esac

done
