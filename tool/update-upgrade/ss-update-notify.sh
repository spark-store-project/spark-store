#!/bin/bash
set -e
isexistnumber=0
if [ "$1" = "now" ];then
	if [  -f /tmp/spark-store-updatenum/number ];then
	update_app_number=`cat /tmp/spark-store-updatenum/number`
	echo "$update_app_number"
		if [ "$update_app_number" = "0" ];then
		exit 0
		fi
	notify-send -i spark-store "星火更新提醒" "星火商店仓库中有$update_app_number个软件包可以更新啦！请到星火商店的菜单处理"
	fi
exit 0
fi

until [  $isexistnumber -eq 1 ];do
if [  -f /tmp/spark-store-updatenum/number ];then
	sleep 10
	update_app_number=`cat /tmp/spark-store-updatenum/number`
	echo "$update_app_number"
		if [ "$update_app_number" = "0" ];then
		exit 0
		fi
	notify-send -i spark-store "星火更新提醒" "星火商店仓库中有$update_app_number个软件包可以更新啦！请到星火商店的菜单处理"
	isexistnumber=1
fi
sleep 10
done
