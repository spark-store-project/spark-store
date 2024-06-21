# Have to disable cleaning for this folder because cmake deletes .ts files upon clean
# Not sure what else wont clean up / dirty workaround of Qt bug
# @ref https://bugreports.qt.io/browse/QTBUG-41736
# @ref https://stackoverflow.com/a/24245615/1917249
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM TRUE)

find_package(Qt6 REQUIRED COMPONENTS LinguistTools)

# Update ts files and release qm files only in Release build
file(GLOB TS_FILES "*.ts")
if (NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_SOURCE_DIR})
    qt6_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${TS_FILES} OPTIONS -no-obsolete)
    # https://stackoverflow.com/questions/70665191/cmake-does-not-generate-ts-files
    add_custom_target(${PROJECT_NAME}_translations DEPENDS ${TS_FILES} ${QM_FILES})
    add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_translations)
endif ()

install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/translations)
