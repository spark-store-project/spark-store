#-------------------------------------------------
#
# Project created by QtCreator 2021-10-29T16:58:36
#
#-------------------------------------------------

QT       += core gui concurrent dbus network svg webenginewidgets

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

# Get build version from qmake
VERSION = $$BUILD_VERSION
isEmpty(VERSION): VERSION = 4.0.0
DEFINES += APP_VERSION=\\\"'$${VERSION}'\\\"
DEFINES += APP_BRANCH=\\\"'$$system(git symbolic-ref --short -q HEAD)'\\\"
# Disable qWarning / qDebug output in Release
#CONFIG(release, debug | release): DEFINES += QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT

CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkcore dtkgui dtkwidget

SOURCES += \
        backend/DataCollectorAndUploader.cpp \
        backend/ThemeChecker.cpp \
        backend/downloadworker.cpp \
        backend/image_show.cpp \
        backend/sparkapi.cpp \
        dbus/dbussparkstoreservice.cpp \
        pages/appintopage.cpp \
        pages/applistpage.cpp \
        pages/settingspage.cpp \
        utils/httprequest.cpp \
        utils/utils.cpp \
        utils/widgetanimation.cpp \
        widgets/base/basewidgetopacity.cpp \
        widgets/common/downloaditem.cpp \
        widgets/common/progressbutton.cpp \
        widgets/common/smoothlistwidget.cpp \
        widgets/common/smoothscrollbar.cpp \
        widgets/common/webenginepage.cpp \
        widgets/common/webengineview.cpp \
        widgets/big_image.cpp \
        widgets/downloadlistwidget.cpp \
        application.cpp \
        main.cpp \
        mainwindow-dtk.cpp

HEADERS += \
        backend/DataCollectorAndUploader.h \
        backend/ThemeChecker.h \
        backend/downloadworker.h \
        backend/image_show.h \
        backend/sparkapi.h \
        dbus/dbussparkstoreservice.h \
        pages/appintopage.h \
        pages/applistpage.h \
        pages/settingspage.h \
        utils/httprequest.h \
        utils/utils.h \
        utils/widgetanimation.h \
        widgets/base/basewidgetopacity.h \
        widgets/common/downloaditem.h \
        widgets/common/progressbutton.h \
        widgets/common/smoothlistwidget.h \
        widgets/common/smoothscrollbar.h \
        widgets/common/webenginepage.h \
        widgets/common/webengineview.h \
        widgets/big_image.h \
        widgets/downloadlistwidget.h \
        application.h \
        mainwindow-dtk.h

FORMS += \
        pages/appintopage.ui \
        pages/applistpage.ui \
        pages/settingspage.ui \
        widgets/common/downloaditem.ui \
        widgets/downloadlistwidget.ui \
        mainwindow-dtk.ui

RESOURCES += \
        assets/assets.qrc

TRANSLATIONS += \
        ../translations/spark-store_en.ts \
        ../translations/spark-store_es.ts \
        ../translations/spark-store_fr.ts \
        ../translations/spark-store_zh_CN.ts \
        ../translations/spark-store_zh_TW.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/durapps/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
