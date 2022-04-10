#!/bin/bash
set -e


echo "可更新的应用有："
sudo apt list --upgradable -o Dir::Etc::sourcelist="sources.list.d/sparkstore.list"     -o Dir::Etc::sourceparts="-" -o APT::Get::List-Cleanup="0"
