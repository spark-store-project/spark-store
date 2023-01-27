#!/bin/bash


endloop=0

#####################检测是否启动过了更新检测工具
while [ $endloop -eq 0 ] ;do

if [  -e /etc/systemd/system/multi-user.target.wants/spark-update-notifier.service ];then 
text_update_open="关闭"
#已经开启了就显示关闭
else
text_update_open="开启"
fi



option=`zenity --list --text="欢迎使用星火更新和安装设置工具\n请在以下操作中选择一个进行~" --column 数字 --column=操作选项  --print-column=2  --height 350 --width 760  0 "$text_update_open星火更新检测工具(如果开启则会在系统启动后自动检测更新。如有更新则会弹出通知)" 1 查看可更新软件包列表并决定是否更新  2 退出脚本 --hide-column=1 --print-column=1` 

case $option in 
	0)
	if [  -e /etc/systemd/system/multi-user.target.wants/spark-update-notifier.service ];then 
	zenity --info --icon-name=spark-store --height 150 --width 200 --text "检测到已经启动了自动更新检测，执行关闭" --timeout=2 
	pkexec systemctl disable spark-update-notifier
	zenity --info --icon-name=spark-store --height 150 --width 200 --text "已关闭" --timeout=2
	else
	zenity --info --icon-name=spark-store --height 150 --width 200 --text  "未检测到自动更新检测，执行启动" --timeout=2 
	pkexec systemctl enable spark-update-notifier
	pkexec service spark-update-notifier start
	zenity --info --icon-name=spark-store --height 150 --width 200 --text "已启动" --timeout=2
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
