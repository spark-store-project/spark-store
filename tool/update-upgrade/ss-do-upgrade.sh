#!/bin/bash
DEPEND=`which apt-fast`
if [ "$DEPEND" = "" ] ; then
echo "没有安装apt-fast，使用apt运行"
sudo apt upgrade -o Dir::Etc::sourcelist="sources.list.d/sparkstore.list"     -o Dir::Etc::sourceparts="-" -o APT::Get::List-Cleanup="0"
else
echo "已安装apt-fast，使用apt-fast加速运行"
ss-apt-fast upgrade -o Dir::Etc::sourcelist="sources.list.d/sparkstore.list"     -o Dir::Etc::sourceparts="-" -o APT::Get::List-Cleanup="0"
fi