#!/bin/bash
set -e
echo "因为目前没有有时间的Qt程序员志愿者所以现在加功能只有让shenmo写脚本了..."
echo
echo "此功能被添加的原因是：为了防止影响系统更新，星火源默认的优先级是400，低于默认的500。这导致了与系统仓库相同包名的软件包无法正常更新到比系统仓库更高的版本。例如：hugo，系统仓库中的低版本会被优先安装，而且默认不会更新到星火的更新的版本"
echo "此脚本可以检测星火源中是否有比当前系统更高版本的软件包，由您决定是否安装"
echo "此脚本使用apt update/apt list --upgrade/apt upgrade来更新软件源、列出更新列表和操作更新"
echo
echo "该脚本不对其使用效果有任何明示或暗示的担保，在使用前请您确保您清楚地知道您在干什么。您应当清楚，星火商店的软件包并不是适配所有发行版的；尽管绝大多数的软件更新是无害的，您仍然应该仔细检查显示出来的更新内容，以确保不会对您的依赖环境造成破坏，尤其是那些看起来和系统依赖相关的软件包。在查看过更新列表之后，确认更新之前，您仍有一次取消更新的机会。"
echo "本脚本的开发者不对使用该脚本可能造成的后果负责"
echo
echo
echo
echo "请按回车继续..."
#"只有回车可以用啊！""啊？"“你说的任意都行啊”“唔。。。戳到我的盲区了”
read

reset

endloop=0


while [ $endloop -eq 0 ] ;do

if [  -f /etc/xdg/autostart/spark-update-notifier.desktop ];then 
textupdateopen="关闭"
#已经开启了就显示关闭
else
textupdateopen="开启"
fi

echo "欢迎使用星火更新工具 脚本版 "
echo "请在以下操作中选择一个进行~"
echo "输入1 $textupdateopen星火更新检测工具(如果开启则会在系统启动后自动检测更新。如有更新则会弹出通知)"
echo "输入2 查看可更新软件包列表并决定是否更新"
echo "输入3 退出脚本"
read option

case $option in 
	1)
	echo "执行以下操作需要授权..."
	if [  -f /etc/xdg/autostart/spark-update-notifier.desktop ];then 
	echo "检测到已经启动了自动更新检测，关闭中"
	sudo systemctl disable spark-update-notifier
	sudo rm /etc/xdg/autostart/spark-update-notifier.desktop
	
	else
	echo "未检测到自动更新检测，启动中"
	sudo systemctl enable spark-update-notifier
	sudo service spark-update-notifier start
#	ln -s /opt/durapps/spark-store/bin/update-upgrade/autostart/spark-update-notifier.desktop $XDG_CONFIG_HOME/autostart
#	
	sudo ln -s /opt/durapps/spark-store/bin/update-upgrade/autostart/spark-update-notifier.desktop /etc/xdg/autostart
	/opt/durapps/spark-store/bin/update-upgrade/ss-update-notify.sh now
	fi
	;;

	2)
	echo "执行以下操作需要授权..."
	/opt/durapps/spark-store/bin/update-upgrade/ss-upgrade-list.sh
	/opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade.sh

	;;
	3)
	echo "按回车退出"
	exit 0
	;;
	*) 
	echo "无法识别的输入！请重新输入"
	reset
esac

done