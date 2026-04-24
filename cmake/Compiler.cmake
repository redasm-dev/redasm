set_target_properties(${PROJECT_NAME}
    PROPERTIES
        EXPORT_COMPILE_COMMANDS ON
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        CXX_STANDARD 17
)

target_compile_features(${PROJECT_NAME}
    PRIVATE
        cxx_std_17
)

set(NOT_MSVC_COMPILE_OPTIONS
    "-Wall"
    "-Wextra"
    "-Wpedantic"
    "-Wno-error=unused"
    "-Wno-error=unused-function"
    "-Wno-error=unused-parameter"
    "-Wno-error=unused-value"
    "-Wno-error=unused-variable"
    "-Wno-error=unused-local-typedefs"
    "-Wno-error=unused-but-set-parameter"
    "-Wno-error=unused-but-set-variable"
    "-fno-rtti"
)

set(NOT_MSVC_LINK_OPTIONS)

set(MSVC_COMPILE_OPTIONS
    "/W4"
)

set(MSVC_LINK_OPTIONS)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND NOT_MSVC_COMPILE_OPTIONS 
        "-Werror"
        "-g"
        "-fsanitize=address,undefined"
        "-fno-omit-frame-pointer"
    )

    list(APPEND NOT_MSVC_LINK_OPTIONS
        "-fsanitize=address,undefined"
        "-fno-omit-frame-pointer"
    )

    list(APPEND MSVC_COMPILE_OPTIONS
        "/WX"
        "/Od"
        "/fsanitize=address"
    )
endif()

target_compile_options(${PROJECT_NAME}
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:${MSVC_COMPILE_OPTIONS}>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:${NOT_MSVC_COMPILE_OPTIONS}>
)

target_link_options(${PROJECT_NAME}
    PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>:${MSVC_LINK_OPTIONS}>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:${NOT_MSVC_LINK_OPTIONS}>
)
