#!/bin/bash

echo "以上可升级，是否升级？[y/n]"
read yes_or_no
if [ "$yes_or_no" = "y" ];then ###这是确定是否同意

echo "获取更新列表..."
PKG_LIST="$(bwrap --dev-bind / / --bind '/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/sparkstore.list' /etc/apt/sources.list.d/sparkstore.list apt list --upgradable -o Dir::Etc::sourcelist="sources.list.d/sparkstore.list"     -o Dir::Etc::sourceparts="-" -o APT::Get::List-Cleanup="0" | awk 'BEGIN {FS="/"} {print $1}' | awk NR\>1)"


  for PKG_NAME in $PKG_LIST;
  do
    echo "$PKG_NAME 正在更新..."
    if sudo aptss install "$PKG_NAME" -y >/dev/null 2>&1;
    then
      echo "$PKG_NAME 已更新"
    else
      echo "WARNING: $PKG_NAME 无法更新"
    fi
  done



fi  ###这是确定是否同意