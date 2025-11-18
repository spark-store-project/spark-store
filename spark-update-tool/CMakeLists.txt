cmake_minimum_required(VERSION 3.16)

project(spark-update-tool VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Network Concurrent)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
        src/aptssupdater.h
        src/aptssupdater.cpp
        src/icons.qrc
        src/appdelegate.h
        src/appdelegate.cpp
        src/applistmodel.h
        src/applistmodel.cpp
        src/downloadmanager.h
        src/downloadmanager.cpp
        src/ignoreconfig.h
        src/ignoreconfig.cpp
)

if(ANDROID)
    add_library(spark-update-tool SHARED
        ${PROJECT_SOURCES}
    )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
else()
    add_executable(spark-update-tool
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(spark-update-tool PRIVATE Qt5::Widgets Qt5::Network Qt5::Concurrent)

set_target_properties(spark-update-tool PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.spark-update-tool
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS spark-update-tool
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)