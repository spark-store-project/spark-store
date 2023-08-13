#!/bin/bash
LANGUAGE=en_US

source /opt/durapps/spark-store/bin/bashimport/transhell.amber
load_transhell_debug

#############################################################


# 发送通知
function notify-send() {


    # Detect user using the display
    local user=$(who | awk '{print $1}' | head -n 1)

    # Detect uid of the user
    local uid=$(id -u $user)

    sudo -u $user  DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/${uid}/bus notify-send "$@"
}



# 检测网络链接畅通
function network-check()
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

network-check
if [ $? -ne 0 ] ; then
    echo "$TRANSHELL_CONTENT_NETWORK_FAIL"
    exit -1
fi

# The code above is modified from https://blog.csdn.net/yaxuan88521/article/details/120516298

# 每日更新星火源文件

aptss update

updatetext=`aptss ssupdate 2>&1`

until [ "`echo $updatetext | grep E: `" = "" ];do
echo "${TRANSHELL_CONTENT_UPDATE_ERROR_AND_WAIT_15_SEC}"
sleep 15
updatetext=`aptss ssupdate 2>&1`



done

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
update_transhell

## 如果都是hold或者版本一致的那就直接退出，否则把剩余的给提醒了

##TODO:除了apt-mark hold之外额外有一个禁止检查列表

## 如果不想提示就不提示

user=$(who | awk '{print $1}' | head -n 1)
if [ -e "/home/$user/.config/spark-union/spark-store/ssshell-config-do-not-show-upgrade-notify" ];then
echo "他不想站在世界之巅，好吧"
echo "Okay he don't want to be at the top of the world, okay"
exit 
else
notify-send -a spark-store "${TRANSHELL_CONTENT_SPARK_STORE_UPGRADE_NOTIFY}" "${TRANSHELL_CONTENT_THERE_ARE_APPS_TO_UPGRADE}"
fi
