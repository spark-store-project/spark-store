#!/bin/bash


script_dir=$(cd $(dirname $0);pwd)

CURDIR=$(dirname $script_dir)

mkdir -p $CURDIR/debian/spark-store/opt/durapps/spark-store/lib
so_list=$(ldd $CURDIR/debian/spark-store/opt/durapps/spark-store/bin/spark-store | awk '{if (match($3, "/")){ print $3}}')

cp -L -n $so_list $CURDIR/debian/spark-store/opt/durapps/spark-store/lib

# so_list=$(ls /lib/x86_64-linux-gnu/libQt5*.so)
# cp -L -n $so_list $CURDIR/debian/spark-store/opt/durapps/spark-store/lib


mkdir -p $CURDIR/debian/spark-store/opt/durapps/spark-store/plugins
cp -L -r /usr/lib/x86_64-linux-gnu/qt5/plugins/platforms $CURDIR/debian/spark-store/opt/durapps/spark-store/plugins >/dev/null || :
cp -L -r /usr/lib/x86_64-linux-gnu/qt5/plugins/wayland-shell-integration $CURDIR/debian/spark-store/opt/durapps/spark-store/plugins >/dev/null || :
cp -L -r /usr/lib/x86_64-linux-gnu/qt5/plugins/iconengines $CURDIR/debian/spark-store/opt/durapps/spark-store/plugins >/dev/null || :
cp -L -r /usr/lib/x86_64-linux-gnu/qt5/plugins/imageformats $CURDIR/debian/spark-store/opt/durapps/spark-store/plugins >/dev/null || :
cp -L -r /usr/lib/x86_64-linux-gnu/qt5/plugins/platformthemes $CURDIR/debian/spark-store/opt/durapps/spark-store/plugins >/dev/null || :
cp -L -r /usr/lib/x86_64-linux-gnu/qt5/plugins/styles $CURDIR/debian/spark-store/opt/durapps/spark-store/plugins >/dev/null || :


# cp -r /usr/share/libdtk*/ $CURDIR/debian/spark-store/opt/durapps/spark-store/ >/dev/null || :
$CURDIR/linuxdeployqt $CURDIR/debian/spark-store/opt/durapps/spark-store/bin/spark-store -appimage >/dev/null || :
rm -rf $CURDIR/debian/spark-store/opt/durapps/AppRun
rm -rf $CURDIR/debian/spark-store/opt/durapps/spark-store/lib/libc.so*