#!/bin/bash
set -e


echo "---可更新的应用有："
sudo bwrap --dev-bind / / --bind '/opt/durapps/spark-store/bin/apt-fast-conf/sources.list.d/sparkstore.list' /etc/apt/sources.list.d/sparkstore.list apt list --upgradable -o Dir::Etc::sourcelist="sources.list.d/sparkstore.list"     -o Dir::Etc::sourceparts="-" -o APT::Get::List-Cleanup="0"
