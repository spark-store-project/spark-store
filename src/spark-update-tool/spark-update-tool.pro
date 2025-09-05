# spark-update-tool.pro
QT       += core gui widgets network concurrent
TARGET = spark-update-tool
TEMPLATE = app

# Set C++ standard to C++17
CONFIG += c++17

# Enable auto features (uic, moc, rcc)
CONFIG += qt warn_on release
QT_CONFIG += no-pkg-config

# Version info (replace with your actual version)
VERSION = 0.1.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# Source files
SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/aptssupdater.cpp \
    src/appdelegate.cpp \
    src/applistmodel.cpp \
    src/downloadmanager.cpp

HEADERS += \
    src/mainwindow.h \
    src/aptssupdater.h \
    src/appdelegate.h \
    src/applistmodel.h \
    src/downloadmanager.h

FORMS += \
    src/mainwindow.ui

RESOURCES += \
    src/icons.qrc

# macOS bundle properties (optional)
macx {
    QMAKE_INFO_PLIST = Info.plist
    ICON = resources/spark-update-tool.icns
    BUNDLE_IDENTIFIER = org.spark.store.update-tool
}

# Installation paths (matches CMake install)
target.path = $$[QT_INSTALL_BINS]
INSTALLS += target