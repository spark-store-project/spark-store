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
    local timeout=5

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


update_app_number=`echo ${updatetext%package*} #从右向左截取第一个 src 后的字符串`
update_app_number=`echo ${update_app_number##*information...}`


notify-send -i spark-store "星火更新提醒" "星火商店仓库中有$update_app_number个软件包可以更新啦！请到星火商店的菜单处理"


