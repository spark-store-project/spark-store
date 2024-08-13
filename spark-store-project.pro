#-------------------------------------------------
#
# Project created by QtCreator 2022-01-12T04:00:00
#
#-------------------------------------------------

TARGET = spark-store
TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
        src/$${TARGET}.pro

# https://wiki.debian.org/Hardening
QMAKE_CFLAGS   *= $(shell dpkg-buildflags --get CFLAGS)
QMAKE_CPPFLAGS *= $(shell dpkg-buildflags --get CPPFLAGS)
QMAKE_CXXFLAGS *= $(shell dpkg-buildflags --get CXXFLAGS)
QMAKE_LFLAGS   *= $(shell dpkg-buildflags --get LDFLAGS)

# Update translation files
CONFIG(release, debug | release) {
    system(bash $${PWD}/translate_update.sh)
    system(bash $${PWD}/translate_generation.sh)
}

# Rules for deployment
tool.files += tool/*
tool.path = /opt/durapps/$${TARGET}/bin/

qm.files += translations/*.qm
qm.path = /usr/share/$${TARGET}/translations/

#preferences.files += pkg/etc/apt/preferences.d/sparkstore
#preferences.path = /etc/apt/preferences.d/

#sourceslist.files += pkg/etc/apt/sources.list.d/sparkstore.list
#sourceslist.path = /etc/apt/sources.list.d/

bash_completion.files += pkg/usr/share/bash-completion/completions/aptss
bash_completion.path = /usr/share/bash-completion/completions/

desktop.files += pkg/usr/share/applications/*.desktop
desktop.path = /usr/share/applications/

service.files += pkg/usr/lib/systemd/system/spark-update-notifier.service
service.path = /lib/systemd/system/

polkit-1.files += pkg/usr/share/polkit-1/actions/store.spark-app.ss-do-upgrade-worker.policy
polkit-1.path = /usr/share/polkit-1/actions/

icon.files += pkg/usr/share/icons
icon.path = /usr/share/

ssinstall-transhell.files += pkg/usr/share/ssinstall/transhell
ssinstall-transhell.path = /usr/share/ssinstall/

aptss-transhell.files += pkg/usr/share/aptss/transhell
aptss-transhell.path = /usr/share/aptss/

tmp.files += pkg/tmp/spark-store-install/feedback.sh
tmp.path = /tmp/spark-store-install/

INSTALLS += \
        tool \
        qm \
        desktop \
        icon \
        ssinstall-transhell \
	aptss-transhell \
#       sourceslist \
        tmp \
        service \
        bash_completion \
        polkit-1 
