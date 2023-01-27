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

touch /tmp/spark-store/upgradeStatus.txt

# 执行 apt update
pkexec /opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade-worker.sh ssupdate | zenity --progress --auto-close --pulsate --no-cancel --text="${TRANSHELL_CONTENT_UPDATE_CHEKING_PLEASE_WAIT}" --height 70 --width 400 --title="${TRANSHELL_CONTENT_SPARK_STORE_UPGRADE_MODEL}" --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg

if [ -z `cat /tmp/spark-store-app-ssupdate-status.txt` ] ; then
	pkexec /opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade-worker.sh clean-log
else
	zenity --error --text "${TRANSHELL_CONTENT_CHECK_UPDATE_PROCESS_ERROR_PRESS_CONFIRM_TO_CHECK}" --title "${TRANSHELL_CONTENT_SPARK_STORE_UPGRADE_MODEL}" --height 200 --width 350 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
	zenity --text-info --filename=/tmp/spark-store-app-ssupdate-log.txt --checkbox="${TRANSHELL_CONTENT_I_ALREDY_COPIED_THE_LOG_HERE_AND_WILL_USE_IT_TO_FEEDBACK}" --title="${TRANSHELL_CONTENT_FEEDBACK_CAN_BE_FOUND_IN_THE_SETTINGS}" --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
	pkexec /opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade-worker.sh clean-log
    rm -f /tmp/spark-store/upgradeStatus.txt
	exit
fi

# 获取可更新应用列表
PKG_LIST="$(pkexec /opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade-worker.sh upgradable-list)"
## 如果没更新，就弹出不需要更新
if [ -z "$PKG_LIST" ] ; then
	zenity --info --text "${TRANSHELL_CONTENT_NO_NEED_TO_UPGRADE}" --title "${TRANSHELL_CONTENT_SPARK_STORE_UPGRADE_MODEL}" --height 150 --width 300 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
else
	## 获取用户选择的要更新的应用
	### 指定分隔符为 \n
	IFS_OLD="$IFS"
	IFS=$'\n'

	PKG_UPGRADE_LIST=`for line in $PKG_LIST ; do
	PKG_NAME=$(echo $line | awk -F ' ' '{print $1}')
	PKG_NEW_VER=$(echo $line | awk -F ' ' '{print $2}')
	PKG_CUR_VER=$(echo $line | awk -F ' ' '{print $3}')

	dpkg --compare-versions $PKG_NEW_VER le $PKG_CUR_VER
	if [ $? -eq 0 ] ; then
		continue
	fi
	#### 检测是否是 hold 状态
	PKG_STA=$(dpkg-query -W -f='${db:Status-Want}' $PKG_NAME)
	if [ "$PKG_STA" != "hold" ] ; then
		echo "true"
		echo "$PKG_NAME"
		echo "$PKG_NEW_VER"
		echo "$PKG_CUR_VER"
	else
		echo "false"
		echo "$PKG_NAME${TRANSHELL_CONTENT_CAN_NOT_UPGRADE_FOR_BEING_HOLD}"
		echo "$PKG_NEW_VER"
		echo "$PKG_CUR_VER"
	fi
done`

	### 还原分隔符
	IFS="$IFS_OLD"

	## 如果没有应用需要更新，则直接退出
	if [ -z "$PKG_UPGRADE_LIST" ] ; then
		zenity --info --text "${TRANSHELL_CONTENT_NO_NEED_TO_UPGRADE}" --title "${TRANSHELL_CONTENT_SPARK_STORE_UPGRADE_MODEL}" --height 150 --width 300 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
	else
		PKG_UPGRADE_LIST=$(echo "$PKG_UPGRADE_LIST" | zenity --list --text="${TRANSHELL_CONTENT_CHOOSE_APP_TO_UPGRADE}" --column="${TRANSHELL_CONTENT_CHOOSE}" --column="${TRANSHELL_CONTENT_PKG_NAME}" --column="${TRANSHELL_CONTENT_NEW_VERSION}" --column="${TRANSHELL_CONTENT_UPGRADE_FROM}" --separator=" " --checklist --multiple --print-column=2 --height 350 --width 550 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg)
		## 如果没有选择，则直接退出
		if [ -z "$PKG_UPGRADE_LIST" ] ; then
			zenity --info --text "${TRANSHELL_CONTENT_NO_APP_IS_CHOSEN}" --title "${TRANSHELL_CONTENT_SPARK_STORE_UPGRADE_MODEL}" --height 150 --width 300 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
		else
			### 更新用户选择的应用
	for PKG_UPGRADE in $PKG_UPGRADE_LIST;do
			update_transhell
			pkexec /opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade-worker.sh upgrade-app $PKG_UPGRADE -y | zenity --progress --auto-close --no-cancel --pulsate --text="${TRANSHELL_CONTENT_UPGRADING_PLEASE_WAIT}" --height 70 --width 400 --title="${TRANSHELL_CONTENT_SPARK_STORE_UPGRADE_MODEL}" --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
	done
			#### 更新成功
			if [ -z "`cat /tmp/spark-store-app-upgrade-status.txt`" ] ; then
				zenity --info --text "${TRANSHELL_CONTENT_CHOSEN_APP_UPGRADE_FINISHED}" --title "${TRANSHELL_CONTENT_SPARK_STORE_UPGRADE_MODEL}" --height 150 --width 300 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
			else
			#### 更新异常
				zenity --error --text "${TRANSHELL_CONTENT_APP_UGRADE_PROCESS_ERROR_PRESS_CONFIRM_TO_CHECK}" --title "${TRANSHELL_CONTENT_SPARK_STORE_UPGRADE_MODEL}" --height 200 --width 350 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
				zenity --text-info --filename=/tmp/spark-store-app-upgrade-log.txt --checkbox="${TRANSHELL_CONTENT_I_ALREDY_COPIED_THE_LOG_HERE_AND_WILL_USE_IT_TO_FEEDBACK}" --title="${TRANSHELL_CONTENT_FEEDBACK_CAN_BE_FOUND_IN_THE_SETTINGS}" --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
			fi
		fi
	fi
fi

rm -f  /tmp/spark-store/upgradeStatus.txt
# 从最开头
