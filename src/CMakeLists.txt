find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Concurrent Network Svg WebEngineWidgets)
find_package(Dtk6 REQUIRED COMPONENTS Core Gui Widget)

include(src.cmake)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${QRC_FILE})

target_include_directories(${PROJECT_NAME} PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Concurrent
    Qt6::Network
    Qt6::Svg
    Qt6::WebEngineWidgets
    Dtk6::Core
    Dtk6::Gui
    Dtk6::Widget
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Concurrent
    Qt6::Network
    Qt6::Svg
    Qt6::WebEngineWidgets
    Dtk6::Core
    Dtk6::Gui
    Dtk6::Widget
)

install(TARGETS ${PROJECT_NAME} DESTINATION /opt/durapps/${PROJECT_NAME}/bin)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/tool/ DESTINATION /opt/durapps/${PROJECT_NAME}/bin USE_SOURCE_PERMISSIONS)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/pkg/${CMAKE_INSTALL_SYSCONFDIR}/aptss DESTINATION ${CMAKE_INSTALL_SYSCONFDIR} USE_SOURCE_PERMISSIONS)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/pkg/lib/ DESTINATION /lib USE_SOURCE_PERMISSIONS)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/pkg/tmp/ DESTINATION /tmp USE_SOURCE_PERMISSIONS)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/pkg/usr/ DESTINATION ${CMAKE_INSTALL_PREFIX} USE_SOURCE_PERMISSIONS)
