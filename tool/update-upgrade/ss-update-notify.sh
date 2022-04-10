#!/bin/bash
set -e

if [  -f /tmp/spark-store-updatenum/number ];then
	update_app_number=`cat /tmp/spark-store-updatenum/number`
	echo "$update_app_number"
	notify-send -i spark-store "星火更新提醒" "星火商店仓库中有$update_app_number个软件包可以更新啦！请及时处理"
fi
