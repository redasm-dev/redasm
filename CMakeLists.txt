cmake_minimum_required(VERSION 3.25)

project(redasm-gui)

if(NOT TARGET redasm::redasm)
    find_package(redasm REQUIRED)
endif()

include(cmake/Dependencies.cmake)
setup_dependencies()

qt_add_executable(${PROJECT_NAME})

qt_add_resources(${PROJECT_NAME} "resources"
    PREFIX /

    FILES
        res/fonts/FontAwesomeBrands6.otf
        res/fonts/FontAwesomeSolid6.otf
        res/themes/light.json
        res/themes/dark.json
        res/logo_dark.png
        res/logo.png
)

include(cmake/Compiler.cmake)
setup_compiler(${PROJECT_NAME})

target_sources(${PROJECT_NAME} 
    PRIVATE
        src/delegates/recentfiles.cpp
        src/dialogs/about.cpp
        src/dialogs/analyzer.cpp
        src/dialogs/decoder.cpp
        src/dialogs/detail.cpp
        src/dialogs/flc.cpp
        src/dialogs/goto.cpp
        src/dialogs/loader.cpp
        src/dialogs/memorymap.cpp
        src/dialogs/segmentregs.cpp
        src/dialogs/settings.cpp
        src/dialogs/table.cpp
        src/hexview/flagsbuffer.cpp
        src/hexview/flagsdelegate.cpp
        src/models/exported.cpp
        src/models/functions.cpp
        src/models/imported.cpp
        src/models/mappings.cpp
        src/models/problems.cpp
        src/models/recentfiles.cpp
        src/models/references.cpp
        src/models/segmentregs.cpp
        src/models/segments.cpp
        src/models/symbols.cpp
        src/models/symbolsfilter.cpp
        src/support/actions.cpp
        src/support/fontawesome.cpp
        src/support/settings.cpp
        src/support/surfacerenderer.cpp
        src/support/themeprovider.cpp
        src/support/utils.cpp
        src/views/graph/node.cpp
        src/views/graph/view.cpp
        src/views/split/view.cpp
        src/views/split/widget.cpp
        src/views/surface/graph/graph.cpp
        src/views/surface/graph/node.cpp
        src/views/surface/listing.cpp
        src/views/surface/path.cpp
        src/views/surface/popup.cpp
        src/views/surface/split.cpp
        src/views/surface/view.cpp
        src/views/context.cpp
        src/views/dashboard.cpp
        src/views/log.cpp
        src/views/welcome.cpp
        src/main.cpp
        src/mainwindow.cpp
        src/statusbar.cpp
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
      SHARED_OBJECT_EXT="${CMAKE_SHARED_LIBRARY_SUFFIX}"
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt6::Widgets
        redasm::redasm
        QHexView
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        WIN32_EXECUTABLE ON
        MACOSX_BUNDLE ON
)

