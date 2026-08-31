include(cmake/CPM.cmake)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

function(setup_dependencies)
    CPMAddPackage(
        NAME QHexView
        # VERSION "5.1.3"
        GIT_TAG "master"
        GITHUB_REPOSITORY "Dax89/QHexView"

        OPTIONS 
            "QHEXVIEW_BUILD_EXAMPLE OFF"
            "QHEXVIEW_ASAN ON"
    )
endfunction()
