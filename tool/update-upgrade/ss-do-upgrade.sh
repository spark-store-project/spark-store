#!/bin/bash

echo "以上可升级，是否升级？[y/n]"
read yes_or_no
if [ "$yes_or_no" = "y" ];then ###这是确定是否同意

echo "获取更新列表..."
PKG_LIST="$(bwrap --dev-bind / / --bind '/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/sparkstore.list' /etc/apt/sources.list.d/sparkstore.list apt list --upgradable -o Dir::Etc::sourcelist="sources.list.d/sparkstore.list"     -o Dir::Etc::sourceparts="-" -o APT::Get::List-Cleanup="0" | awk 'BEGIN {FS="/"} {print $1}' | awk NR\>1)"


isuos=`cat /etc/os-release | grep UnionTech`
if [ "$isuos" != "" ]; then ###这是确定是否为UOS 如果是
echo "UOS中系统依赖无法使用第三方下载工具，使用apt-fast下载本体"

  cd /var/cache/apt/archives
  for PKG_NAME in $PKG_LIST;
  do
    echo "$PKG_NAME 正在下载..."
  sudo aptss download "$PKG_NAME" -y >/dev/null 2>&1;
  done
   
  echo "----开始安装"
  for PKG_NAME in $PKG_LIST;
  do
    echo "$PKG_NAME 正在准备更新..."
    if sudo bwrap --dev-bind / / --bind '/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/sparkstore.list' /etc/apt/sources.list.d/sparkstore.list apt install "$PKG_NAME" -y   >/dev/null 2>&1;
    then
      echo "$PKG_NAME 已更新"
    else
      echo "WARNING: $PKG_NAME 无法更新"
    fi
  done



else ###这是确定是否为UOS

  for PKG_NAME in $PKG_LIST;
  do
    echo "$PKG_NAME 正在准备更新..."
    if sudo aptss install "$PKG_NAME" -y >/dev/null 2>&1;
    then
      echo "$PKG_NAME 已更新"
    else
      echo "WARNING: $PKG_NAME 无法更新"
    fi
  done


fi ###这是确定是否为UOS

fi  ###这是确定是否同意