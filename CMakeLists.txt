cmake_minimum_required(VERSION 3.10)
project(spark_store)
set(EXECUTABLE_NAME spark-store)

# Begin Compilation Options

# When set to ON, DTK Plugin for DDE platform will be built.
# Note that only machines with DTK and dev packages installed can build the plugin.
set(SPARK_BUILD_DTK_PLUGIN ON)

# When set to ON, store will transmit telemetry even in Debug builds.
set(SPARK_FORCE_TELEMETRY OFF)

# End Compilation Options

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QT_VERSION 5)
set(REQUIRED_LIBS
    Core
    Gui
    Widgets
    Network
    Concurrent)
set(REQUIRED_LIBS_QUALIFIED
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    Qt5::Concurrent)

include_directories(inc)
include_directories(plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PRE_CONFIGURE_FILE "src/gitver.cpp.in")
set(POST_CONFIGURE_FILE "${CMAKE_CURRENT_BINARY_DIR}/src/gitver.cpp")
include(cmake/git_watcher.cmake)
add_library(gitver STATIC ${POST_CONFIGURE_FILE})
#target_include_directories(gitver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_dependencies(gitver check_git)

set(SOURCE_FILES
    src/main.cpp

    resource/resource.qrc

    inc/gitver.h
    inc/deepinplatform.h
    inc/dtk/spkdtkplugin.h
    inc/spkutils.h src/spkutils.cpp
    inc/telemetry/collectid.h
    inc/spkqsshelper.h gui/spkqsshelper.cpp

    inc/qt/elidedlabel.h gui/qt/elidedlabel.cpp

    inc/spkwindow.h gui/spkwindow.cpp
    inc/spktitlebar.h gui/spktitlebar.cpp
    inc/spkui_general.h gui/spkui_general.cpp
    inc/spkmainwindow.h gui/spkmainwindow.cpp
    inc/spkmsgbox.h gui/spkmsgbox.cpp
    inc/spkdialog.h gui/spkdialog.cpp
    inc/spkabout.h gui/spkabout.cpp
    inc/spkloading.h gui/spkloading.cpp
    inc/spksidebartree.h gui/spksidebartree.cpp
    inc/spkappitem.h gui/spkappitem.cpp
    inc/spkpopup.h gui/spkpopup.cpp
    inc/spkstretchlayout.h gui/spkstretchlayout.cpp
    inc/spkfocuslineedit.h

    inc/page/spkpagebase.h gui/page/spkpagebase.cpp
    inc/page/spkpageuitest.h gui/page/spkpageuitest.cpp
    inc/page/spkpageapplist.h gui/page/spkpageapplist.cpp
    inc/page/spkpageappdetails.h gui/page/spkpageappdetails.cpp

    inc/spkstore.h src/spkstore.cpp
    inc/spkuimsg.h src/spkuimsg.cpp
    inc/spklogging.h src/spklogging.cpp
    inc/spkresource.h src/spkresource.cpp
    inc/spkdownload.h src/spkdownload.cpp
    )

include(cmake/FindLibNotify.cmake)
include(cmake/FindGlib.cmake)
include(cmake/FindGdk3.cmake)

include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GDK3_INCLUDE_DIRS})
set(LIBLINKING ${LIBLINKING}
    gitver
    ${LIBNOTIFY_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${GDK3_LIBRARIES}
    ${CMAKE_DL_LIBS})

# Required for a good backtrace
add_compile_options(-g)
add_link_options(-rdynamic)

# Find Qt before adding other build targets
find_package(Qt${QT_VERSION} COMPONENTS ${REQUIRED_LIBS} REQUIRED)

add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES})

if(SPARK_BUILD_DTK_PLUGIN)
    add_subdirectory(plugin/dtkplugin)
    add_dependencies(${EXECUTABLE_NAME} spkdtkplugin)
endif()

target_link_libraries(${EXECUTABLE_NAME} ${REQUIRED_LIBS_QUALIFIED} ${LIBLINKING})

install(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION bin)
