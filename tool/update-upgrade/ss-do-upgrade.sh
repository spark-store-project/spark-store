#!/bin/bash

touch /tmp/spark-store/upgradeStatus.txt

# 执行 apt update
pkexec /opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade-worker.sh ssupdate | zenity --progress --auto-close --pulsate --no-cancel --text="正在检查更新，请稍候..." --height 70 --width 400 --title="星火商店更新模块" --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg

if [ -z `cat /tmp/spark-store-app-ssupdate-status.txt` ] ; then
	echo "无错误"
	pkexec /opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade-worker.sh clean-log
else
	zenity --error --text "检查更新进程出现错误！按确定查看报错，可用于反馈" --title "星火商店更新检测模块" --height 200 --width 350 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
	zenity --text-info --filename=/tmp/spark-store-app-ssupdate-log.txt --checkbox="我已复制了此文本框中的日志，且将会在反馈时附上。反馈渠道可以在右上角菜单的设置中找到" --title="反馈渠道在商店右上角的设置里" --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
	pkexec /opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade-worker.sh clean-log
    rm -f /tmp/spark-store/upgradeStatus.txt
	exit
fi

# 获取可更新应用列表
PKG_LIST="$(pkexec /opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade-worker.sh upgradable-list)"
## 如果没更新，就弹出不需要更新
if [ -z "$PKG_LIST" ] ; then
	zenity --info --text "没有软件需要更新\n但是你并没有站在世界之巅" --title "星火商店更新检测模块" --height 150 --width 300 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
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
		echo "$PKG_NAME（无法更新：已被标记为保留）"
		echo "$PKG_NEW_VER"
		echo "$PKG_CUR_VER"
	fi
done`

	### 还原分隔符
	IFS="$IFS_OLD"

	## 如果没有应用需要更新，则直接退出
	if [ -z "$PKG_UPGRADE_LIST" ] ; then
		zenity --info --text "没有软件需要更新\n但是你并没有站在世界之巅" --title "星火商店更新检测服务" --height 150 --width 300 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
	else
		PKG_UPGRADE_LIST=$(echo "$PKG_UPGRADE_LIST" | zenity --list --text="选择你想更新的应用" --column="是否更新" --column="包名" --column="新版本" --column="从该版本更新" --separator=" " --checklist --multiple --print-column=2 --height 350 --width 550 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg)
		## 如果没有选择，则直接退出
		if [ -z "$PKG_UPGRADE_LIST" ] ; then
			zenity --info --text "没有选中任何软件\n但是你并没有站在世界之巅" --title "星火商店更新检测服务" --height 150 --width 300 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
		else
			### 更新用户选择的应用
			pkexec /opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade-worker.sh upgrade-app $PKG_UPGRADE_LIST -y | zenity --progress --auto-close --no-cancel --pulsate --text="正在更新已选中的应用，请稍候..." --height 70 --width 400 --title="星火商店更新模块" --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg

			#### 更新成功
			if [ -z "`cat /tmp/spark-store-app-upgrade-status.txt`" ] ; then
				zenity --info --text "选中的软件已经更新完毕" --title "星火商店更新检测模块" --height 150 --width 300 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
			else
			#### 更新异常
				zenity --error --text "更新出现错误！按确定查看报错，可用于反馈" --title "星火商店更新检测模块" --height 200 --width 350 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
				zenity --text-info --filename=/tmp/spark-store-app-upgrade-log.txt --checkbox="我已复制了此文本框中的日志，且将会在反馈时附上 。反馈渠道可以在右上角菜单的设置中找到" --title="反馈渠道在商店右上角的设置里往下拉" --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
			fi
		fi
	fi
fi

rm -f  /tmp/spark-store/upgradeStatus.txt
# 从最开头
