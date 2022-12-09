#-------------------------------------------------
#
# Project created by QtCreator 2022-01-12T04:00:00
#
#-------------------------------------------------

TARGET = spark-store
TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
        src/spark-store.pro

TRANSLATIONS += \
        translations/spark-store_fr.ts \
        translations/spark-store_en.ts \
        translations/spark-store_zh_CN.ts

# Update translation files
CONFIG(release, debug|release): system(bash $${PWD}/translate_generation.sh)

# Rules for deployment
tool.files += tool/*
tool.path = /opt/durapps/$${TARGET}/bin

qm.files += translations/*.qm
qm.path = /usr/share/spark-store/translations

preferences.files += pkg/etc/apt/preferences.d/sparkstore
preferences.path = /etc/apt/preferences.d

sourceslist.files += pkg/etc/apt/sources.list.d/sparkstore.list
sourceslist.path = /etc/apt/sources.list.d

desktop.files += pkg/usr/share/applications/spark-store.desktop
desktop.path = /usr/share/applications

icon.files += pkg/usr/share/icons/hicolor/scalable/apps/spark-store.svg
icon.path = /usr/share/icons/hicolor/scalable/apps

tmp.files += pkg/tmp/spark-store-install/feedback.sh
tmp.path = /tmp/spark-store-install

INSTALLS += \
        tool \
        qm \
        desktop \
        icon \
        sourceslist \
        preferences \
        tmp
