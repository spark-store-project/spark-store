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

if [  -f /usr/share/polkit-1/actions/store.spark-app.ssinstall.policy ];then 
text_auto_install_open="关闭"
#已经开启了就显示关闭
else
text_auto_install_open="开启"
fi

option=`zenity --list --text="欢迎使用星火更新和安装设置工具\n请在以下操作中选择一个进行~" --column 数字 --column=操作选项  --print-column=2  --height 350 --width 760 0 "查看自动更新相关功能使用前须知（重要）" 1 "$text_update_open星火更新检测工具(如果开启则会在系统启动后自动检测更新。如有更新则会弹出通知)" 2 查看可更新软件包列表并决定是否更新 3 "$text_auto_install_open点击安装免输入密码功能" 4 退出脚本 --hide-column=1 --print-column=1` 

case $option in 
	0)
	zenity --info --icon-name=spark-store --height 450 --width 500 --text "此须知适用于自动更新功能\n\n此脚本可以检测星火源中是否有比当前系统更高版本的软件包，由您决定是否安装；在安装更新过程中请不要直接点击关闭本页面，以免发生错误\n\n此脚本使用apt update/apt list --upgrade/apt upgrade来更新软件源、列出更新列表和操作更新\n\n该脚本不对其使用效果有任何明示或暗示的担保，在使用前请您确保您清楚地知道您在干什么。您应当清楚，星火商店的软件包并不是适配所有发行版的；尽管绝大多数的软件更新是无害的，您仍然应该仔细检查显示出来的更新内容，以确保不会对您的依赖环境造成破坏，尤其是那些看起来和系统依赖相关的软件包。在查看过更新列表之后，确认更新之前，您仍有取消更新的机会。\n\n本脚本的开发者不对使用该脚本可能造成的后果负责"
	;;
	1)
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

	2)
	/opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade.sh
	;;
	3)
	if [ -f /usr/share/polkit-1/actions/store.spark-app.ssinstall.policy ];then 
	zenity --info --icon-name=spark-store --height 150 --width 200 --text   "---检测到已经启动了免输入密码，执行关闭" --timeout=2 
	pkexec rm /usr/share/polkit-1/actions/store.spark-app.ssinstall.policy
	zenity --info --icon-name=spark-store --height 150 --width 200 --text   "已关闭" --timeout=2 
	else

	is_accept_polkiy=`zenity --list  --height 350 --width 300 --text "请注意：这个功能尚未开发完成，一旦开启，则运行pkexec ssinstall时不再需要授权\n仅对星火内置安装器生效\n理论上会存在一定的安全风险" --column 数字 --column=操作选项 --hide-column=1 --print-column=1 1 同意 2 拒绝`



if [ "$is_accept_polkiy" = "1" ];then
	pkexec ln -s /opt/durapps/spark-store/bin/auto-install-policy/store.spark-app.ssinstall.policy /usr/share/polkit-1/actions/store.spark-app.ssinstall.policy
	zenity --info --icon-name=spark-store --height 150 --width 200 --text  "---已启动"
	else
	zenity --info --icon-name=spark-store --height 150 --width 200 --text  "---未同意，返回"
fi
fi
	;;
	4)
	exit 0
	;;
	*) 
	exit 0
esac

done