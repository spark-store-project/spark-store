#-------------------------------------------------
#
# Project created by QtCreator 2019-06-30T12:53:03
#
#-------------------------------------------------

QT       += core gui network concurrent webenginewidgets dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = spark-store
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += QT_APP_DEBUG
include(../third-party/QtNetworkService/QtNetworkService.pri)

CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkcore dtkgui dtkwidget libnotify

# 禁止输出 qWarning / qDebug 信息
#CONFIG(release, debug|release): DEFINES += QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT

SOURCES += \
    appitem.cpp \
    big_image.cpp \
    downloadlist.cpp \
    downloadworker.cpp \
    flowlayout.cpp \
    image_show.cpp \
    main.cpp \
    progressload.cpp \
    widget.cpp \
    workerthreads.cpp \
    dbus/dbussparkstore.cpp \
    webengine/webenginepage.cpp \
    webengine/webengineview.cpp

HEADERS += \
    appitem.h \
    big_image.h \
    downloadlist.h \
    downloadworker.h \
    flowlayout.h \
    image_show.h \
    progressload.h \
    widget.h \
    workerthreads.h \
    dbus/dbussparkstore.h \
    webengine/webenginepage.h \
    webengine/webengineview.h

FORMS += \
    appitem.ui \
    downloadlist.ui \
    widget.ui

RESOURCES += \
    ../assets/icons.qrc

DISTFILES += \
    ../assets/tags/a2d-small.png \
    ../assets/tags/a2d.png \
    ../assets/tags/community-small.png \
    ../assets/tags/community.png \
    ../assets/tags/deepin-small.png \
    ../assets/tags/dtk-small.png \
    ../assets/tags/ubuntu-small.png \
    ../assets/tags/ubuntu.png \
    ../assets/tags/uos-small.png \
    ../assets/tags/community.svg \
    ../assets/tags/deepin.svg \
    ../assets/tags/logo_icon.svg \
    ../assets/tags/uos.svg

TRANSLATIONS += \
    ../translations/spark-store_en.ts \
    ../translations/spark-store_fr.ts \
    ../translations/spark-store_zh_CN.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/durapps/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
