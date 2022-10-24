#!/bin/bash
set -e
LANG=en.US

function notify-send() {
    #Detect the name of the display in use
    local display=":$(ls /tmp/.X11-unix/* | sed 's#/tmp/.X11-unix/X##' | head -n 1)"

    #Detect the user using such display
    local user=$(who | grep '('$display')' | awk '{print $1}' | head -n 1)

    #Detect the id of the user
    local uid=$(id -u $user)

    sudo -u $user DISPLAY=$display DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$uid/bus notify-send "$@"
}

#检测网络链接畅通
function network()
{
    #超时时间
    local timeout=15

    #目标网站
    local target=www.baidu.com

    #获取响应状态码
    local ret_code=`curl -I -s --connect-timeout ${timeout} ${target} -w %{http_code} | tail -n1`

    if [ "x$ret_code" = "x200" ]; then
        #网络畅通
        return 0
    else
        #网络不畅通
        return 1
    fi

    return 0
}

network
if [ $? -eq 1 ];then
	echo "Network fail. Stop to avoid bother dpkg"
	exit -1
fi

#The code above is modified from https://blog.csdn.net/yaxuan88521/article/details/120516298

curl --progress-bar -o /opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/sparkstore.list "https://gitee.com/deepin-community-store/repo_auto_update_script/raw/master/mirror-list-for-apt-fast/sources.list.d/sparkstore.list"
# 每日更新星火源文件
mkdir -p /etc/apt/preferences.d
touch /etc/apt/preferences.d/sparkstore
cat << EOF >/etc/apt/preferences.d/sparkstore
Package: *
Pin: origin *.deepinos.org.cn
Pin-Priority: 100

EOF

updatetext=`aptss ssupdate`

rm /etc/apt/preferences.d/sparkstore


isupdate=`echo ${updatetext: -5}`
if [ "$isupdate" = "date." ];then
exit 0 
fi
#### 从这里开始，只有检测到了更新才会进行

update_app_number=`echo ${updatetext%package*} #从右向左截取第一个 src 后的字符串`
update_app_number=`echo ${update_app_number##*information...}`


PKG_LIST="$(bwrap --dev-bind / / --bind '/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/sparkstore.list' /etc/apt/sources.list.d/sparkstore.list apt list --upgradable -o Dir::Etc::sourcelist="sources.list.d/sparkstore.list"     -o Dir::Etc::sourceparts="-" -o APT::Get::List-Cleanup="0" | awk 'BEGIN {FS="/"} {print $1}' | awk NR\>1)" 



for PKG_NAME in $PKG_LIST;do
if [ "$(dpkg-query -W -f='${Status}' $PKG_NAME | grep hold)" != "" ];then
       let update_app_number=$update_app_number-1
	echo $update_app_number
	echo $PKG_NAME 
fi
done



if [ $update_app_number -lt 1 ];then
exit
fi
#### 如果都是hold的那就直接退出，否则把剩余的给提醒了

notify-send -i spark-store "星火更新提醒" "星火商店仓库中有$update_app_number个软件包可以更新啦！请到星火商店的菜单处理"


