cmake_minimum_required(VERSION 3.14)

project(MaxwellsMargins VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    set(SFML_DIR "${CMAKE_SOURCE_DIR}/vendor/SFML/lib/cmake/SFML")
    enable_language(RC)
elseif(APPLE)
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/sfml")
endif()
find_package(SFML 3.0.2 COMPONENTS Graphics Window System REQUIRED)

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/vendor/imgui")
set(IMGUI_SFML_FIND_SFML OFF)
add_subdirectory(vendor/imgui-sfml)

add_subdirectory(vendor/fmt)

add_subdirectory(vendor/matplotplusplus)

add_subdirectory(vendor/json)

if(WIN32)
    set(APP_ICON_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/resources/windows/app.rc")
elseif(APPLE)
    set(APP_ICON_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/resources/mac/icon.icns")
    set_source_files_properties(${APP_ICON_RESOURCE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/core/App.cpp
    src/core/World.cpp
    src/core/Tools.cpp
    src/engine/Solver.cpp
    src/engine/FieldMath.cpp
    src/electrostatics/FieldSampler.cpp
    src/electrostatics/EquipotentialTracer.cpp
    src/electrostatics/GaussianSurface.cpp
    src/circuits/CircuitGraph.cpp
    src/render/Renderer.cpp
    src/logging/Logger.cpp
    src/graphing/Grapher.cpp
    src/scenes/Presets.cpp
    ${APP_ICON_RESOURCE}
)

if(APPLE)
    # NOTE: the bundle/executable name intentionally has no apostrophe - the Unix
    # Makefiles generator breaks its custom-command shell quoting on paths containing one.
    # The apostrophe still shows correctly to users via CFBundleName (menu bar, About panel, etc).
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "Maxwells Margins"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Maxwell's Margins"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.maxwellsmargins.app"
        MACOSX_BUNDLE_ICON_FILE "icon.icns"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/resources/mac/Info.plist.in"
    )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics SFML::Window SFML::System ImGui-SFML::ImGui-SFML fmt::fmt matplot nlohmann_json::nlohmann_json)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/vendor/SFML/bin"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMENT "Copying SFML DLLs to build directory..."
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -DTARGET_DIR=$<TARGET_FILE_DIR:${PROJECT_NAME}> -DSRC_DIR=${CMAKE_CURRENT_LIST_DIR} -P ${CMAKE_CURRENT_LIST_DIR}/copy_assets.cmake
    COMMENT "Copying assets to build directory..."
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()
