include(GNUInstallDirs)

add_definitions('-g' '-gdwarf-4')

# configuration

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR})

set(HAVE_SPACEWARE ${SPACEWARE_FOUND})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# solvespace dependencies
add_library(slvs_deps INTERFACE)
target_include_directories(slvs_deps INTERFACE SYSTEM
    ${OPENGL_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
    ${PNG_PNG_INCLUDE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
    ${AGG_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIRS})
target_link_libraries(slvs_deps INTERFACE
    dxfrw
    ${ZLIB_LIBRARY}
    ${PNG_LIBRARY}
    ${FREETYPE_LIBRARY}
    ${CAIRO_LIBRARIES})

if(Backtrace_FOUND)
    target_include_directories(slvs_deps INTERFACE SYSTEM
        ${Backtrace_INCLUDE_DIRS})
    target_link_libraries(slvs_deps INTERFACE
        ${Backtrace_LIBRARY})
endif()

if(SPACEWARE_FOUND)
    target_include_directories(slvs_deps INTERFACE SYSTEM
        ${SPACEWARE_INCLUDE_DIR})
    target_link_libraries(slvs_deps INTERFACE
        ${SPACEWARE_LIBRARIES})
endif()

if(ENABLE_OPENMP)
    target_link_libraries(slvs_deps INTERFACE slvs_openmp)
endif()

target_compile_options(slvs_deps
    INTERFACE ${COVERAGE_FLAGS})

# platform utilities

# libslvs
add_library(slvs SHARED
    solvespace.h
    platform/platform.h
    util.cpp
    entity.cpp
    expr.cpp
    constraint.cpp
    constrainteq.cpp
    system.cpp
    platform/platform.cpp
    lib.cpp)

target_compile_definitions(slvs
    PRIVATE -DLIBRARY)

target_include_directories(slvs
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${EIGEN3_INCLUDE_DIRS})

target_link_libraries(slvs PRIVATE slvs_deps)

set_target_properties(slvs PROPERTIES
    PUBLIC_HEADER ${CMAKE_SOURCE_DIR}/include/slvs.h
    VERSION ${PROJECT_VERSION}
    SOVERSION 1)

if(NOT WIN32)
    install(TARGETS slvs
        LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

# solvespace library

set(solvespace_core_gl_SOURCES
    solvespace.cpp)

add_library(solvespace-core STATIC
    dsc.h
    expr.h
    polygon.h
    sketch.h
    solvespace.h
    ui.h
    platform/platform.h
    render/render.h
    render/gl3shader.h
    srf/surface.h
    bsp.cpp
    clipboard.cpp
    confscreen.cpp
    constraint.cpp
    constrainteq.cpp
    describescreen.cpp
    draw.cpp
    drawconstraint.cpp
    drawentity.cpp
    entity.cpp
    export.cpp
    exportstep.cpp
    exportvector.cpp
    expr.cpp
    file.cpp
    generate.cpp
    graphicswin.cpp
    group.cpp
    groupmesh.cpp
    importdxf.cpp
    importidf.cpp
    importmesh.cpp
    mesh.cpp
    modify.cpp
    mouse.cpp
    polyline.cpp
    polygon.cpp
    resource.cpp
    request.cpp
    style.cpp
    system.cpp
    textscreens.cpp
    textwin.cpp
    toolbar.cpp
    ttf.cpp
    undoredo.cpp
    util.cpp
    view.cpp
    platform/platform.cpp
    platform/gui.cpp
    render/render.cpp
    render/render2d.cpp
    srf/boolean.cpp
    srf/curve.cpp
    srf/merge.cpp
    srf/ratpoly.cpp
    srf/raycast.cpp
    srf/shell.cpp
    srf/surface.cpp
    srf/surfinter.cpp
    srf/triangulate.cpp)

target_link_libraries(solvespace-core PUBLIC slvs_deps)

# solvespace headless library

add_library(solvespace-headless STATIC EXCLUDE_FROM_ALL
    ${solvespace_core_gl_SOURCES}
    platform/guinone.cpp
    render/rendercairo.cpp
    render/renderagg.cpp)

target_compile_definitions(solvespace-headless
    PRIVATE HEADLESS)

target_include_directories(solvespace-headless
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${EIGEN3_INCLUDE_DIRS}
    ${AGG_INCLUDE_DIRS}
    "platform/haiku/ui/")

target_link_libraries(solvespace-headless
    PRIVATE
    agg
    be
    solvespace-core)
