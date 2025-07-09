cmake_minimum_required(VERSION 3.16)

project(spark-update-tool VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt5 REQUIRED COMPONENTS Widgets Network Concurrent)


find_package(Qt5 REQUIRED COMPONENTS Widgets Network Concurrent Core Gui)

# 定义所有项目源文件，现在无需条件判断
set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    src/aptssupdater.h src/aptssupdater.cpp
    src/icons.qrc
    src/appdelegate.h src/appdelegate.cpp
    src/applistmodel.h src/applistmodel.cpp
    src/downloadmanager.h src/downloadmanager.cpp
)


add_executable(spark-update-tool
    ${PROJECT_SOURCES}
)

target_link_libraries(spark-update-tool PRIVATE
    Qt5::Widgets
    Qt5::Network
    Qt5::Concurrent
    Qt5::Core
    Qt5::Gui
)


set_target_properties(spark-update-tool PROPERTIES
    # ${BUNDLE_ID_OPTION} # 如果上面取消注释，这里也取消注释
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}

)

include(GNUInstallDirs)
install(TARGETS spark-update-tool
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

