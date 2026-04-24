include(cmake/CPM.cmake)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

function(setup_dependencies)
    CPMAddPackage("gh:Dax89/QHexView@5.1.0")
endfunction()
