#!/bin/sh

realpath=`realpath $0`
cd `dirname $realpath`

mkdir -p build && cd build

/usr/lib/qt5/bin/qmake ../spark-store-project.pro -spec linux-g++ CONFIG+=qtquickcompiler && /usr/bin/make qmake_all
make -j$(cat /proc/cpuinfo | grep "processor" | wc -l)

cd ..

cp ./ssinstall ./debian/opt/durapps/spark-store/bin/
cp ./spark-dstore-patch ./debian/opt/durapps/spark-store/bin/
cp ./build/src/spark-store ./debian/opt/durapps/spark-store/bin/
cp ./translations/*.qm ./debian/usr/share/spark-store/translations/

version=$(cat ./debian/DEBIAN/control | grep "Version" | awk -F ' '  '{print $2}')

fakeroot dpkg -b ./debian ./
