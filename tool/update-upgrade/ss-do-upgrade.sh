#!/bin/bash
#临时提升星火源的优先级
sed -i 's/400/500/g' /etc/apt/preferences.d/sparkstore
sudo apt upgrade
sed -i 's/500/400/g' /etc/apt/preferences.d/sparkstore
#恢复优先级

