#!/bin/bash
LANGUAGE=en_US

# 发送通知
function notify-send() {
    # Detect name of the display in use
    local display=":$(ls /tmp/.X11-unix/* | sed 's#/tmp/.X11-unix/X##' | head -n 1)"

    # Detect user using the display
    local user=$(who | grep '('$display')' | awk '{print $1}' | head -n 1)

    # Detect uid of the user
    local uid=$(id -u $user)

    sudo -u $user DISPLAY=$display DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/${uid}/bus notify-send "$@"
}

# 检测网络链接畅通
function network()
{
    # 超时时间
    local timeout=15

    # 目标网站
    local target=www.baidu.com

    # 获取响应状态码
    local ret_code=`curl -I -s --connect-timeout ${timeout} ${target} -w %{http_code} | tail -n1`

    if [ "x$ret_code" = "x200" ] ; then
        # 网络畅通
        return 0
    else
        # 网络不畅通
        return 1
    fi
}

network
if [ $? -ne 0 ] ; then
    echo "Network fail. Stop to avoid bother dpkg"
    exit -1
fi

# The code above is modified from https://blog.csdn.net/yaxuan88521/article/details/120516298

# 每日更新星火源文件
curl --progress-bar -o /opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/sparkstore.list "https://gitee.com/deepin-community-store/repo_auto_update_script/raw/master/mirror-list-for-apt-fast/sources.list.d/sparkstore.list"

updatetext=`aptss ssupdate`
isupdate=`echo ${updatetext: -5}`
if [ "$isupdate" = "date." ] ; then
    exit 0
fi

## 从这里开始，只有检测到了更新才会进行
update_app_number=`echo ${updatetext%package*} #从右向左截取第一个 src 后的字符串`
update_app_number=`echo ${update_app_number##*information...}`

# 获取用户选择的要更新的应用
PKG_LIST="$(/opt/durapps/spark-store/bin/update-upgrade/ss-do-upgrade-worker.sh upgradable-list)"
# 指定分隔符为 \n
IFS_OLD="$IFS"
IFS=$'\n'

for line in $PKG_LIST ; do
    PKG_NAME=$(echo $line | awk -F ' ' '{print $1}')
    PKG_NEW_VER=$(echo $line | awk -F ' ' '{print $2}')
    PKG_CUR_VER=$(echo $line | awk -F ' ' '{print $3}')

    dpkg --compare-versions $PKG_NEW_VER le $PKG_CUR_VER

    if [ $? -eq 0 ] ; then
        let update_app_number=$update_app_number-1
        continue
    fi

    ## 检测是否是 hold 状态
    PKG_STA=$(dpkg-query -W -f='${db:Status-Want}' $PKG_NAME)
    if [ "$PKG_STA" = "hold" ] ; then
        let update_app_number=$update_app_number-1
    fi
done

# 还原分隔符
IFS="$IFS_OLD"

if [ $update_app_number -le 0 ] ; then
    exit 0
fi

## 如果都是hold或者版本一致的那就直接退出，否则把剩余的给提醒了
notify-send -a spark-store "星火更新提醒" "星火商店仓库中有$update_app_number个软件包可以更新啦！请到星火商店的菜单处理"
