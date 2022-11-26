TARGET = spark-store
TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
    src/spark-dstore-patch \
    third-party/QtNetworkService \
    src/spark-store.pro

spark-store.depends = third-party/QtNetworkService

# Update translation files
CONFIG(release, debug|release): system(bash $${PWD}/translate_generation.sh)

# Rules for deployment
tool.files += tool/*
tool.path = /opt/durapps/$${TARGET}/bin

qm.files += translations/*.qm
qm.path = /usr/share/spark-store/translations

#preferences.files += pkg/etc/apt/preferences.d/sparkstore
#preferences.path = /etc/apt/preferences.d

#sourceslist.files += pkg/etc/apt/sources.list.d/sparkstore.list
#sourceslist.path = /etc/apt/sources.list.d

bash_completion.files += pkg/usr/share/bash-completion/completions/aptss
bash_completion.path = /usr/share/bash-completion/completions

desktop.files += pkg/usr/share/applications/spark-store.desktop
desktop.path = /usr/share/applications

service.files += pkg/usr/lib/systemd/system/spark-update-notifier.service
service.path = /usr/lib/systemd/system/

polkit-1.files +=pkg/usr/share/polkit-1/actions/store.spark-app.ss-do-upgrade-worker.policy
polkit-1.path = /usr/share/polkit-1/actions/

icon.files += pkg/usr/share/icons/hicolor/scalable/apps/spark-store.svg
icon.path = /usr/share/icons/hicolor/scalable/apps

tmp.files += pkg/tmp/spark-store-install/feedback.sh
tmp.path = /tmp/spark-store-install

INSTALLS += \
    tool \
    qm \
    desktop \
    icon \
#    preferences \
#    sourceslist \
    tmp \
    service \
    bash_completion \
    polkit-1 
#   暂时不添加
