cmake_minimum_required(VERSION 3.9)
project(plugin VERSION 1.5.0 DESCRIPTION "INAV-X-Plane-HITL-Plugin")

set(OUTPUT_DIR CACHE STRING "Full path to xplanes plugin directory")

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10" CACHE STRING "Minimum macOS version" FORCE)
set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architectures for mac OS X" FORCE)

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif ()

set(CMAKE_CXX_STANDARD 17)

set(PLUGIN_SRC_DIR ${CMAKE_SOURCE_DIR}/src)

set(PLUGIN_SOURCES
    ${PLUGIN_SRC_DIR}/main.cpp
    ${PLUGIN_SRC_DIR}/graph.cpp
    ${PLUGIN_SRC_DIR}/lodepng.cpp
    ${PLUGIN_SRC_DIR}/menu.cpp
    ${PLUGIN_SRC_DIR}/msp.cpp
    ${PLUGIN_SRC_DIR}/osd.cpp
    ${PLUGIN_SRC_DIR}/serialbase.cpp
    ${PLUGIN_SRC_DIR}/serial.cpp
    ${PLUGIN_SRC_DIR}/tcpserial.cpp
    ${PLUGIN_SRC_DIR}/simData.cpp
    ${PLUGIN_SRC_DIR}/stats.cpp
    ${PLUGIN_SRC_DIR}/util.cpp
    ${PLUGIN_SRC_DIR}/sound.cpp
    ${PLUGIN_SRC_DIR}/map.cpp
    ${PLUGIN_SRC_DIR}/fontbase.cpp
    ${PLUGIN_SRC_DIR}/fontanalog.cpp
    ${PLUGIN_SRC_DIR}/fontwalksnail.cpp
    ${PLUGIN_SRC_DIR}/fonthdzero.cpp
    ${PLUGIN_SRC_DIR}/fontwtfos.cpp
    ${PLUGIN_SRC_DIR}/ipinputwidget.cpp
)

include_directories(
    "${PLUGIN_SRC_DIR}/SDK/CHeaders/Widgets"
    "${PLUGIN_SRC_DIR}/SDK/CHeaders/Wrappers"
    "${PLUGIN_SRC_DIR}/SDK/CHeaders/XPLM"
    "${PLUGIN_SRC_DIR}/mINI"
    "${PLUGIN_SRC_DIR}/OpenAL/include"
)

if (WIN32)
    list(APPEND CMAKE_LIBRARY_PATH "${PLUGIN_SRC_DIR}/SDK/Libraries/Win")
elseif (APPLE)
    list(APPEND CMAKE_FRAMEWORK_PATH "${PLUGIN_SRC_DIR}/SDK/Libraries/Mac")
elseif (UNIX)

endif ()

add_definitions(-DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1 -DXPLM302=1 -DXPLM303=1)
add_definitions(-DAPL=$<BOOL:${APPLE}> -DIBM=$<BOOL:${WIN32}> -DLIN=$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>>)
add_compile_options(-fexceptions)
add_compile_options(-fvisibility=hidden)
add_compile_options(-Wall -Wpedantic -Wextra)
add_compile_options(-Wno-unused)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O3 -fPIC)
endif ()

if (APPLE)
    add_compile_options(-Wno-nested-anon-types)
    add_compile_options(-Wno-unused-parameter)
    add_compile_options(-Wno-deprecated-declarations)
endif()

add_library(plugin SHARED ${PLUGIN_SOURCES})

target_compile_features(plugin PUBLIC cxx_std_17)


if (APPLE)
    target_compile_options(plugin PUBLIC -mmacosx-version-min=11.3)
    target_link_libraries(plugin -mmacosx-version-min=11.3)
endif ()

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_library(OPENAL_LIBRARY NAMES openal OpenAL OpenAL32)
find_library(GLUT_LIBRARY NAMES glut GLUT glut64) 
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED IMPORTED_TARGET "gtk+-3.0")


if (WIN32 OR APPLE)
    find_library(XPLM_LIBRARY NAMES XPLM XPLM_64.lib)
	find_library(XPWIDGETS_LIBRARY NAMES XPWidgets XPWidgets_64.lib)
    target_link_libraries(plugin ${XPLM_LIBRARY} ${XPWIDGETS_LIBRARY})
endif ()


if (UNIX)
    find_library(DL_LIBRARY dl)
    target_link_libraries(plugin ${DL_LIBRARY} PkgConfig::GTK)
endif ()

if (APPLE)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    find_library(GLUT_LIBRARY GLUT)
    find_library(OpenGL_LIBRARY OpenGL)
    find_library(Cocoa_LIBRARY Cocoa)
    find_library(OpenAL_Library OpenAL)

    target_link_libraries(plugin ${IOKIT_LIBRARY} ${CORE_FOUNDATION_LIBRARY} ${GLUT_LIBRARY})
    target_link_libraries(plugin ${OpenGL_LIBRARY} ${Cocoa_LIBRARY} ${OpenAL_Library})
endif ()

if (WIN32)
    target_link_libraries(plugin -static-libgcc -static-libstdc++ opengl32)
    target_link_libraries(plugin -static)
elseif (APPLE)
    target_link_libraries(plugin "-exported_symbols_list ${PLUGIN_SRC_DIR}/exports.sym_mac")
	target_link_libraries(plugin "-framework XPLM -framework XPWidgets -ldl")
elseif (UNIX)
    target_link_libraries(plugin -Wl,--version-script -Wl,${PLUGIN_SRC_DIR}/exports.txt)
endif ()

set_target_properties(plugin PROPERTIES PREFIX "")
if (WIN32)
    set_target_properties(plugin PROPERTIES OUTPUT_NAME "win")
elseif (APPLE)
    set_target_properties(plugin PROPERTIES OUTPUT_NAME "mac")
elseif (UNIX)
    set_target_properties(plugin PROPERTIES OUTPUT_NAME "lin")
endif ()
set_target_properties(plugin PROPERTIES SUFFIX ".xpl")

if (NOT ${OUTPUT_DIR} STREQUAL "")
    add_custom_command(TARGET plugin
                        POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:plugin> ${OUTPUT_DIR}
    )
endif()