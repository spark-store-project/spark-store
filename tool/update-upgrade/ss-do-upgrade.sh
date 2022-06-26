#!/bin/bash

echo "以上可升级，是否升级？[y/n]"
read yes_or_no
if [ "$yes_or_no" = "y" ];then
sudo ss-apt-fast upgrade -y -o Dir::Etc::sourcelist="sources.list.d/sparkstore.list"     -o Dir::Etc::sourceparts="-" -o APT::Get::List-Cleanup="0"
else
exit
fi