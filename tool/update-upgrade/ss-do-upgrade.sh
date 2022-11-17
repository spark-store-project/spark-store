#!/bin/bash

touch /tmp/spark-store/upgradeStatus.txt
pkexec /opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade-worker.sh ssupdate | zenity --progress --auto-close --pulsate --no-cancel  --text="正在检查更新，请稍候..." --height 70 --width 400 --title="星火商店更新模块" --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg

if [ -z `cat /tmp/spark-store-app-ssupdate-status.txt` != "0" ];then
echo "无错误"
else
zenity --error  --text "检查更新进程出现错误！按确定查看报错，可用于反馈" --title "星火商店更新检测模块" --height 200 --width 350 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
zenity --text-info --filename=/tmp/spark-store-app-ssupdate-log.txt --checkbox="我已复制了此文本框中的日志，且将会在反馈时附上 。反馈渠道可以在右上角菜单的设置中找到" --title="反馈渠道在商店右上角的设置里" --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
pkexec /opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade-worker.sh clean-log
exit 
fi
pkexec /opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade-worker.sh clean-log

PKG_LIST="$(pkexec /opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade-worker.sh upgradable-list)" 
####如果没更新，就弹出不需要更新
if [ -z "$PKG_LIST" ];then
zenity --info  --text "没有软件需要更新\n但是你并没有站在世界之巅" --title "星火商店更新检测模块" --height 150 --width 300 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
else
PKG_UPGRADE_LIST=`for PKG_NAME in $PKG_LIST;
do
#### 检测是否是hold状态
	if [ "$(dpkg-query -W -f='${Status}' $PKG_NAME | grep hold)" = "" ];then
       echo "true"
       echo "$PKG_NAME"
       echo "$PKG_NAME"
	else
	echo "false"
	echo "$PKG_NAME（无法更新：已被标记为保留）"
	echo "$PKG_NAME"
	fi
done | zenity --list --text="选择你想更新的应用" --column=是否更新 --column=应用包名 --column="真的应用包名" --separator=" " --checklist --print-column=3 --multiple --height 350 --width 550 --hide-column=3 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg` 

#### 如果没有选择，则直接退出

if [ "$PKG_UPGRADE_LIST" = "" ];then
zenity --info  --text "没有选中任何软件\n但是你并没有站在世界之巅" --title "星火商店更新检测服务" --height 150 --width 300 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
else

pkexec /opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade-worker.sh upgrade-app $PKG_UPGRADE_LIST -y | zenity --progress --auto-close --no-cancel --pulsate --text=正在更新已选中的应用，请稍候... --height 70 --width 400 --title="星火商店更新模块" --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg

if [ -z "`cat /tmp/spark-store-app-upgrade-status.txt`"  ];then

zenity --info  --text "选中的软件已经更新完毕" --title "星火商店更新检测模块" --height 150 --width 300 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
else
zenity --error  --text "更新出现错误！按确定查看报错，可用于反馈" --title "星火商店更新检测模块" --height 200 --width 350 --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
zenity --text-info --filename=/tmp/spark-store-app-upgrade-log.txt --checkbox="我已复制了此文本框中的日志，且将会在反馈时附上 。反馈渠道可以在右上角菜单的设置中找到" --title="反馈渠道在商店右上角的设置里往下拉" --window-icon=/usr/share/icons/hicolor/scalable/apps/spark-store.svg
fi



fi

fi

rm -f touch /tmp/spark-store/upgradeStatus.txt
####从最开头
