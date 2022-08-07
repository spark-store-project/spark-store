#!/bin/bash
set -e
rm -rf /tmp/spark-store-updatenum/number
LANG=en.US

#!/bin/bash

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

updatetext=`ss-apt-fast ssupdate`


mkdir -p /tmp/spark-store-updatenum/


isupdate=`echo ${updatetext: -5}`
if [ "$isupdate" = "date." ];then
sudo echo "0" > /tmp/spark-store-updatenum/number
exit 0 
fi


update_app_number=`echo ${updatetext%package*} #从右向左截取第一个 src 后的字符串`
update_app_number=`echo ${update_app_number##*information...}`
sudo echo "$update_app_number" > /tmp/spark-store-updatenum/number

	

