#!/bin/bash

if [ "$(id -u)" != "0" ] ; then
	if [ "$IS_ACE_ENV" = "1" ] ; then
		/opt/durapps/spark-store/bin/store-helper/pass-auth.sh "$0" "$@"
	else
		xhost +
		pkexec "$0" "$@"
		exit
	fi
fi
HERE=$(dirname $0)
trap "rm -f /tmp/spark-store/upgradeStatus.txt" EXIT
source /opt/durapps/spark-store/bin/bashimport/transhell.amber
load_transhell_debug

function get_name_from_desktop_file() {
	local app_name_in_desktop
	local name_orig
	local name_i18n
	local package_name
	package_name=$1
	for desktop_file_path in $(dpkg -L "$package_name" |grep /usr/share/applications/ | awk '/\.desktop$/ {print}') ; do
		if [ "$(grep -m 1 '^NoDisplay=' "$desktop_file_path" | cut -d '=' -f 2)" = "true" ] ||  [ "$(grep -m 1 '^NoDisplay=' "$desktop_file_path" | cut -d '=' -f 2)" = "True" ] ; then
			continue
		else
			name_orig=$(awk -F= '/^\[Desktop Entry\]$/ {found=1} found && /^Name=/ {print $2; exit} /^\[.*\]$/ && !/\[Desktop Entry\]/ {exit}' "$desktop_file_path")
			name_i18n=$(awk -v lang="Name[$LANGUAGE]" -F= '/^\[Desktop Entry\]$/ {found=1} found && /^Name\[/ && $1 == lang {print $2; exit} /^\[.*\]$/ && !/\[Desktop Entry\]/ {exit}' "$desktop_file_path")
			if [ -z "$name_i18n" ] ; then
				app_name_in_desktop=$name_orig
			else
				app_name_in_desktop=$name_i18n
			fi
		fi
    done

	for desktop_file_path in $(dpkg -L "$package_name" |grep /opt/apps/$package_name/entries/applications | awk '/\.desktop$/ {print}') ; do
		if [ "$(grep -m 1 '^NoDisplay=' "$desktop_file_path" | cut -d '=' -f 2)" = "true" ] ||  [ "$(grep -m 1 '^NoDisplay=' "$desktop_file_path" | cut -d '=' -f 2)" = "True" ] ; then
			continue
		else
			name_orig=$(awk -F= '/^\[Desktop Entry\]$/ {found=1} found && /^Name=/ {print $2; exit} /^\[.*\]$/ && !/\[Desktop Entry\]/ {exit}' "$desktop_file_path")
			name_i18n=$(awk -v lang="Name[$LANGUAGE]" -F= '/^\[Desktop Entry\]$/ {found=1} found && /^Name\[/ && $1 == lang {print $2; exit} /^\[.*\]$/ && !/\[Desktop Entry\]/ {exit}' "$desktop_file_path")
			if [ -z "$name_i18n" ] ; then
				app_name_in_desktop=$name_orig
			else
				app_name_in_desktop=$name_i18n
			fi
		fi
    done

	if [ -z "$app_name_in_desktop" ] ; then
		app_name_in_desktop=${package_name}
	fi

	echo ${app_name_in_desktop}
}

touch /tmp/spark-store/upgradeStatus.txt

# 执行 apt update
pkexec ${HERE}/ss-do-upgrade-worker.sh ssupdate 2>&1 > /dev/null | zenity --progress --auto-close --pulsate --no-cancel --text="${TRANSHELL_CONTENT_UPDATE_CHEKING_PLEASE_WAIT}" --height 70 --width 400 --title="${TRANSHELL_CONTENT_SPARK_STORE_UPGRADE_MODEL}" --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg

if [ -z `cat /tmp/spark-store-app-ssupdate-status.txt` ] ; then
	${HERE}/ss-do-upgrade-worker.sh clean-log
else
	zenity --error --text "${TRANSHELL_CONTENT_CHECK_UPDATE_PROCESS_ERROR_PRESS_CONFIRM_TO_CHECK}" --title "${TRANSHELL_CONTENT_SPARK_STORE_UPGRADE_MODEL}" --height 200 --width 350 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
	zenity --text-info --filename=/tmp/spark-store-app-ssupdate-log.txt --checkbox="${TRANSHELL_CONTENT_I_ALREDY_COPIED_THE_LOG_HERE_AND_WILL_USE_IT_TO_FEEDBACK}" --title="${TRANSHELL_CONTENT_FEEDBACK_CAN_BE_FOUND_IN_THE_SETTINGS}" --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
	${HERE}/ss-do-upgrade-worker.sh clean-log
    rm -f /tmp/spark-store/upgradeStatus.txt
	exit
fi

# 获取可更新应用列表
PKG_LIST="$(${HERE}/ss-do-upgrade-worker.sh upgradable-list)"
## 如果没更新，就弹出不需要更新
if [ -z "$PKG_LIST" ] ; then
	zenity --info --text "${TRANSHELL_CONTENT_NO_NEED_TO_UPGRADE}" --title "${TRANSHELL_CONTENT_SPARK_STORE_UPGRADE_MODEL}" --height 150 --width 300 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
else
	## 获取用户选择的要更新的应用
	### 指定分隔符为 \n
	IFS_OLD="$IFS"
	IFS=$'\n'

	PKG_UPGRADE_LIST=$(for line in $PKG_LIST ; do
	PKG_NAME=$(echo $line | awk -F ' ' '{print $1}')
	PKG_NEW_VER=$(echo $line | awk -F ' ' '{print $2}')
	PKG_CUR_VER=$(echo $line | awk -F ' ' '{print $3}')

	dpkg --compare-versions $PKG_NEW_VER le $PKG_CUR_VER
	if [ $? -eq 0 ] ; then
		continue
	fi
	APP_NAME=$(get_name_from_desktop_file $PKG_NAME)
	#### 检测是否是 hold 状态
	PKG_STA=$(dpkg-query -W -f='${db:Status-Want}' $PKG_NAME)
	if [ "$PKG_STA" != "hold" ] ; then
		echo "true"
		echo "$APP_NAME"
		echo "$PKG_NEW_VER"
		echo "$PKG_CUR_VER"
		echo "$PKG_NAME"
	else
		echo "false"
		echo "$APP_NAME${TRANSHELL_CONTENT_CAN_NOT_UPGRADE_FOR_BEING_HOLD}"
		echo "$PKG_NEW_VER"
		echo "$PKG_CUR_VER"
		echo "$PKG_NAME"
	fi
done)

	### 还原分隔符
	IFS="$IFS_OLD"

	## 如果没有应用需要更新，则直接退出
	if [ -z "$PKG_UPGRADE_LIST" ] ; then
		zenity --info --text "${TRANSHELL_CONTENT_NO_NEED_TO_UPGRADE}" --title "${TRANSHELL_CONTENT_SPARK_STORE_UPGRADE_MODEL}" --height 150 --width 300 
        exit 0
	fi


    while true;do
		PKG_UPGRADE_LIST=$(echo "$PKG_UPGRADE_LIST" | zenity --list --text="${TRANSHELL_CONTENT_CHOOSE_APP_TO_UPGRADE}" --column="${TRANSHELL_CONTENT_CHOOSE}" --column="${TRANSHELL_CONTENT_APP_NAME}" --column="${TRANSHELL_CONTENT_NEW_VERSION}" --column="${TRANSHELL_CONTENT_UPGRADE_FROM}" --column="${TRANSHELL_CONTENT_PKG_NAME}" --separator=" " --checklist --multiple --print-column=5 --height 350 --width 650 )
		## 如果没有选择，则直接退出
		if [ -z "$PKG_UPGRADE_LIST" ] ; then
			zenity --info --text "${TRANSHELL_CONTENT_NO_APP_IS_CHOSEN}" --title "${TRANSHELL_CONTENT_SPARK_STORE_UPGRADE_MODEL}" --height 150 --width 300 
             exit 0
		fi
        if [[ "$PKG_UPGRADE_LIST" == *"(null)"* ]]; then
            zenity --error --text "${TRANSHELL_CONTENT_LIST_NOT_LOADED_PLEASE_WAIT}" --title "${TRANSHELL_CONTENT_SPARK_STORE_UPGRADE_MODEL}" --height 150 --width 300
        else
            break
        fi
done
			### 更新用户选择的应用
#	for PKG_UPGRADE in $PKG_UPGRADE_LIST;do
#			APP_UPGRADE="$(get_name_from_desktop_file $PKG_UPGRADE)"
#			update_transhell

(
total=$(echo "$PKG_UPGRADE_LIST" | wc -w)
count=0

for PKG_UPGRADE in $PKG_UPGRADE_LIST; do
    count=$((count + 1))
    APP_UPGRADE="$(get_name_from_desktop_file $PKG_UPGRADE)"
    update_transhell

    # 启动升级任务
    (yes n | pkexec ${HERE}/ss-do-upgrade-worker.sh upgrade-app $PKG_UPGRADE -yfq 2>&1 > /dev/null ) &

    # 计算进度百分比
    progress=$(( count * 100 / total - 1))
    
    # 动态修改zenity的文本
    echo "# ${TRANSHELL_CONTENT_UPGRADING_PLEASE_WAIT}"
    echo "$progress"
    wait
done
) | zenity --progress --auto-close --no-cancel --text="Preparing..." --height 70 --width 400 --title="${TRANSHELL_CONTENT_SPARK_STORE_UPGRADE_MODEL}" --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg


			#### 更新成功
			if [ -z "`cat /tmp/spark-store-app-upgrade-status.txt`" ] ; then
				zenity --info --text "${TRANSHELL_CONTENT_CHOSEN_APP_UPGRADE_FINISHED}" --title "${TRANSHELL_CONTENT_SPARK_STORE_UPGRADE_MODEL}" --height 150 --width 300 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
			else #### 更新异常
				zenity --error --text "${TRANSHELL_CONTENT_APP_UGRADE_PROCESS_ERROR_PRESS_CONFIRM_TO_CHECK}" --title "${TRANSHELL_CONTENT_SPARK_STORE_UPGRADE_MODEL}" --height 200 --width 350 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
				zenity --text-info --filename=/tmp/spark-store-app-upgrade-log.txt --checkbox="${TRANSHELL_CONTENT_I_ALREDY_COPIED_THE_LOG_HERE_AND_WILL_USE_IT_TO_FEEDBACK}" --title="${TRANSHELL_CONTENT_FEEDBACK_CAN_BE_FOUND_IN_THE_SETTINGS}" --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
			fi
fi

rm -f /tmp/spark-store/upgradeStatus.txt
# 从最开头
