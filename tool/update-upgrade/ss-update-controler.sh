#!/bin/bash

reset

endloop=0

#####################检测是否启动过了更新检测工具
while [ $endloop -eq 0 ] ;do

if [  -f /etc/xdg/autostart/spark-update-notifier.desktop ];then 
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


echo "欢迎使用星火更新和安装设置工具"
echo "请在以下操作中选择一个进行~"
echo "输入0 查看自动更新相关功能使用前须知（重要）"
echo "输入1 $text_update_open星火更新检测工具(如果开启则会在系统启动后自动检测更新。如有更新则会弹出通知)"
echo "输入2 查看可更新软件包列表并决定是否更新"
echo "输入3 $text_auto_install_open点击安装免输入密码功能"
echo "输入4 退出脚本"
echo
read option

case $option in 
	0)
	reset
	echo "此须知适用于自动更新功能"
	echo "此脚本可以检测星火源中是否有比当前系统更高版本的软件包，由您决定是否安装；在安装更新过程中请不要直接点击关闭本页面，以免发生错误"
	echo "此脚本使用apt update/apt list --upgrade/apt upgrade来更新软件源、列出更新列表和操作更新"
	echo
	echo "该脚本不对其使用效果有任何明示或暗示的担保，在使用前请您确保您清楚地知道您在干什么。您应当清楚，星火商店的软件包并不是适配所有发行版的；尽管绝大多数的软件更新是无害的，您仍然应该仔细检查显示出来的更新内容，以确保不会对您	的依赖环境造成破坏，尤其是那些看起来和系统依赖相关的软件包。在查看过更新列表之后，确认更新之前，您仍有一次取消更新的机会。"
	echo "本脚本的开发者不对使用该脚本可能造成的后果负责"
	echo 
	echo 
	echo
	echo "请按回车继续..."
#"只有回车可以用啊！""啊？"“你说的任意都行啊”“唔。。。戳到我的盲区了”
	read
	reset
	;;
	1)
	echo "执行以下操作需要授权..."
	if [  -f /etc/xdg/autostart/spark-update-notifier.desktop ];then 
	echo "---检测到已经启动了自动更新检测，执行关闭"
	sudo systemctl disable spark-update-notifier
	sudo rm /etc/xdg/autostart/spark-update-notifier.desktop
	sleep 3
	reset
	else
	echo "---未检测到自动更新检测，执行启动"
	sudo systemctl enable spark-update-notifier
	sudo service spark-update-notifier start
#	ln -s /opt/durapps/spark-store/bin/update-upgrade/autostart/spark-update-notifier.desktop $XDG_CONFIG_HOME/autostart
#	
	sudo ln -s /opt/durapps/spark-store/bin/update-upgrade/autostart/spark-update-notifier.desktop /etc/xdg/autostart
	/opt/durapps/spark-store/bin/update-upgrade/ss-update-notify.sh now
	sleep 3
	reset
	fi
	;;

	2)
	echo "请注意：如果有些更新需要从系统获取更多依赖，你将需要手动操作 sudo ss-apt-fast upgrade"
	echo "执行以下操作需要授权..."
	/opt/durapps/spark-store/bin/update-upgrade/ss-upgrade-list.sh
	/opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade.sh
	echo "---请按回车返回"
	read
	reset
	;;
	3)
	if [ -f /usr/share/polkit-1/actions/store.spark-app.ssinstall.policy ];then 
	echo "---检测到已经启动了免输入密码，执行关闭"
	sudo rm /usr/share/polkit-1/actions/store.spark-app.ssinstall.policy
	sleep 3
	reset
	else

cat << EOF
请注意：这个功能尚未开发完成，一旦开启，则运行pkexec ssinstall时不再需要授权
仅对星火内置安装器生效
理论上会存在一定的安全风险~
如果接受，请输入1，否则输入2
EOF
read is_accept_polkiy

if [ "$is_accept_polkiy" = "1" ];then
	echo "执行以下操作需要授权..."
	sudo ln -s /opt/durapps/spark-store/bin/auto-install-policy/store.spark-app.ssinstall.policy /usr/share/polkit-1/actions/store.spark-app.ssinstall.policy
	echo "---已启动"
	sleep 3
	reset
	else
	echo "---未同意，返回"
	sleep 3
	reset
fi
fi
	;;
	4)
	exit 0
	;;
	*) 
	echo "---无法识别的输入！请重新输入"
	reset
esac

done