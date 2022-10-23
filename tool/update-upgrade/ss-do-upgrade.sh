#!/bin/bash
aptss ssupdate | zenity --progress --auto-close --no-cancel --pulsate --text=正在更新检查，请稍候... --height 70 --width 400 --title="星火商店更新模块"
PKG_LIST="$(bwrap --dev-bind / / --bind '/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/sparkstore.list' /etc/apt/sources.list.d/sparkstore.list apt list --upgradable -o Dir::Etc::sourcelist="sources.list.d/sparkstore.list"     -o Dir::Etc::sourceparts="-" -o APT::Get::List-Cleanup="0" | awk 'BEGIN {FS="/"} {print $1}' | awk NR\>1)" 
####如果没更新，就弹出不需要更新
if [ -z "$PKG_LIST" ];then
zenity --info --icon-name=spark-store --text "没有软件需要更新\n但是你并没有站在世界之巅" --title "星火商店更新检测服务" --height 150 --width 300
else
PKG_UPGRADE_LIST=`for PKG_NAME in $PKG_LIST;
do
#### 检测是否是hold状态
	if [ "$(dpkg-query -W -f='${Status}' $PKG_NAME | grep hold)" = "" ];then
       echo "true"
       echo "$PKG_NAME"
	else
	echo "false"
	echo "$PKG_NAME (无法更新：请先执行 sudo apt-mark unhold 后再更新)"
	fi
done | zenity --list --text="选择你想更新的应用" --column=是否更新 --column=应用包名 --separator=" " --checklist --print-column=2 --multiple --height 350 --width 550 ` 

#### 如果没有选择，则直接退出

if [ "$PKG_UPGRADE_LIST" = "" ];then
zenity --info --icon-name=spark-store --text "没有选中任何软件\n但是你并没有站在世界之巅" --title "星火商店更新检测服务" --height 150 --width 300
else
pkexec bash aptss install $PKG_UPGRADE_LIST -y | zenity --progress --auto-close --no-cancel --pulsate --text=正在更新已选中的应用，请稍候... --height 70 --width 400 --title="星火商店更新模块"

if [ "$?" = "0" ];then

zenity --info --icon-name=spark-store --text "已选中的软件已经更新完毕" --title "星火商店更新检测服务" --height 150 --width 300
else
zenity --error  --text "更新出现错误！请手动执行 sudo aptss full-upgrade 查看问题" --title "星火商店更新检测服务" --height 150 --width 300
fi

fi

fi
####从最开头