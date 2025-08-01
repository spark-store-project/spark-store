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
