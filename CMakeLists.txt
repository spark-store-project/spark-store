<<<<<<< HEAD
cmake_minimum_required(VERSION 3.12)

project(spark-store)

if(NOT DEFINED VERSION)
    set(VERSION 4.0.0)
endif()
add_compile_definitions(APP_VERSION="${VERSION}")

execute_process(
    COMMAND git symbolic-ref --short -q HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE BRANCH
)
string(STRIP "${BRANCH}" BRANCH)
add_compile_definitions(APP_BRANCH="${BRANCH}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(AUTOMOC_COMPILER_PREDEFINES ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif ()
if (NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif ()

add_compile_definitions(QT_MESSAGELOGCONTEXT)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif()

include(GNUInstallDirs)

add_subdirectory(src)
add_subdirectory(translations)
=======
cmake_minimum_required(VERSION 3.16)

project(spark-update-tool VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Concurrent)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(spark-update-tool
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        src/aptssupdater.h  src/aptssupdater.cpp
        src/icons.qrc
        src/appdelegate.h src/appdelegate.cpp
        src/applistmodel.h src/applistmodel.cpp
        src/downloadmanager.h src/downloadmanager.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET spark-update-tool APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
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
endif()

target_link_libraries(spark-update-tool PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Concurrent)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.spark-update-tool)
endif()
set_target_properties(spark-update-tool PROPERTIES
    ${BUNDLE_ID_OPTION}
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

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(spark-update-tool)
endif()
>>>>>>> update-tool/dev
