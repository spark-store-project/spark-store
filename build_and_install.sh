
# Deepin V20/UOS 21 系统下, 安装依赖

# ```shell
# sudo apt install git qt5-default debhelper pkg-config qtchooser libqt5core5a libqt5gui5 libqt5widgets5 libqt5network5 libqt5concurrent5 libdtkcore-dev libdtkgui-dev  libdtkwidget-dev qttools5-private-dev libnotify-dev qtwebengine5-dev fakeroot qtwayland5 qtwayland5-dev-tools dde-qt5wayland-plugin

# ```

# Ubuntu 22.04 系统下, 安装依赖
# ```shell
# sudo apt install git qtbase5-dev debhelper pkg-config qtchooser libqt5core5a libqt5gui5 libqt5widgets5 libqt5network5 libqt5concurrent5 libdtkcore-dev libdtkgui-dev  libdtkwidget-dev qttools5-private-dev libnotify-dev qtwebengine5-dev qtwayland5 qtwayland5-dev-tools

# ```

echo "Deepin V20/UOS 21 系统下, 安装依赖"
echo "sudo apt install git qt5-default debhelper pkg-config qtchooser libqt5core5a libqt5gui5 libqt5widgets5 libqt5network5 libqt5concurrent5 libdtkcore-dev libdtkgui-dev  libdtkwidget-dev qttools5-private-dev libnotify-dev qtwebengine5-dev fakeroot qtwayland5 qtwayland5-dev-tools dde-qt5wayland-plugin"

echo "Ubuntu 22.04 系统下, 安装依赖"
echo "sudo apt install git qtbase5-dev debhelper pkg-config qtchooser libqt5core5a libqt5gui5 libqt5widgets5 libqt5network5 libqt5concurrent5 libdtkcore-dev libdtkgui-dev  libdtkwidget-dev qttools5-private-dev libnotify-dev qtwebengine5-dev qtwayland5 qtwayland5-dev-tools"

dpkg-buildpackage -j$(cat /proc/cpuinfo | grep processor | wc -l)
sudo apt reinstall ../spark-store_*.deb

rm ../spark-store_*