cmake_minimum_required (VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(ENABLE_COVERAGE ON)

if (ENABLE_COVERAGE)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

project(SolveSpace)

# can't just set this because it'll emit -std=gnu++20 with no way to stop it :grimace:
# set_property(TARGET SolveSpace PROPERTY CXX_STANDARD 20)

add_definitions(-g -gdwarf-4 -fpermissive -Wno-deprecated-enum-float-conversion -Wno-conversion-null -Wno-deprecated-declarations -Wno-deprecated-enum-enum-conversion -Wno-register -std=c++20)

add_subdirectory(res)
add_subdirectory(extlib/libdxfrw)

find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(Freetype REQUIRED)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/config.h
)

add_library(solvespace-core OBJECT
	src/platform/gui.cpp
	src/platform/platform.cpp
	src/importdxf.cpp
	src/exportvector.cpp
	src/ttf.cpp
	src/bsp.cpp
	src/clipboard.cpp
	src/confscreen.cpp
	src/constraint.cpp
	src/constrainteq.cpp
	src/describescreen.cpp
	src/draw.cpp
	src/drawconstraint.cpp
	src/drawentity.cpp
	src/entity.cpp
	src/export.cpp
	src/exportstep.cpp
	src/expr.cpp
	src/file.cpp
	src/generate.cpp
	src/graphicswin.cpp
	src/group.cpp
	src/groupmesh.cpp
    src/importidf.cpp
    src/importmesh.cpp
	src/mesh.cpp
	src/modify.cpp
	src/mouse.cpp
	src/polygon.cpp
	src/polyline.cpp
	src/request.cpp
	src/resource.cpp
	src/style.cpp
	src/system.cpp
	src/textscreens.cpp
	src/textwin.cpp
	src/undoredo.cpp
	src/util.cpp
	src/view.cpp
	src/solvespace.cpp
	src/render/render.cpp
	src/render/render2d.cpp
	src/render/renderagg.cpp
	src/srf/boolean.cpp
	src/srf/curve.cpp
	src/srf/merge.cpp
	src/srf/ratpoly.cpp
	src/srf/raycast.cpp
	src/srf/shell.cpp
	src/srf/surface.cpp
	src/srf/surfinter.cpp
	src/srf/triangulate.cpp
)

target_include_directories(solvespace-core PRIVATE
	${FREETYPE_INCLUDE_DIRS}
	/boot/home/cppfront/include/
	/boot/system/develop/headers/agg2/
	src/
	extlib/libdxfrw/
	${CMAKE_CURRENT_BINARY_DIR} # adds config.h/config.in.h
)

add_executable(SolveSpace
	src/platform/haiku/ui/App.cpp
	src/platform/haiku/ui/MainWindow.cpp
	src/platform/haiku/ui/Toolbar.cpp
	src/platform/haiku/ui/PropertyBrowser.cpp
	src/platform/haiku/ui/ListView/DragSortableListView.cpp
	src/platform/haiku/ui/ListView/ThumbListItem.cpp
	src/platform/haiku/ui/GroupsListView.cpp
	src/platform/haiku/ui/HVIFUtil.cpp
	src/platform/haiku/ui/EditorView.cpp
	src/platform/haiku/ui/ViewParameters.cpp
	src/platform/haiku/ui/guihaiku.cpp
	src/platform/haiku/ui/HaikuSpaceUI.cpp
	$<TARGET_OBJECTS:solvespace-core>
)

target_include_directories(SolveSpace PRIVATE
	/boot/home/cppfront/include/
	/boot/system/develop/headers/agg2/
	src/
	${CMAKE_CURRENT_BINARY_DIR} # adds config.h/config.in.h
)

target_link_libraries(SolveSpace be agg translation tracker dxfrw ${ZLIB_LIBRARY} ${PNG_LIBRARY} ${FREETYPE_LIBRARY})

add_custom_command(TARGET SolveSpace POST_BUILD
	COMMENT "rebuilding icon resources"
	COMMAND rc -o ${CMAKE_BINARY_DIR}/Resources.rsrc ${CMAKE_SOURCE_DIR}/src/platform/haiku/Resources.rdef
	COMMAND xres -o ${CMAKE_BINARY_DIR}/SolveSpace ${CMAKE_BINARY_DIR}/Resources.rsrc
		-a VICN:1:line-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/line-tool.hvif
		-a VICN:2:rect-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/rect-tool.hvif
		-a VICN:3:circle-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/circle-tool.hvif
		-a VICN:4:arc-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/arc-tool.hvif
		-a VICN:5:tangent-arc-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/tangent-arc-tool.hvif
		-a VICN:6:cubic-spline-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/cubic-spline-tool.hvif
		-a VICN:7:datum-point-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/datum-point-tool.hvif
		-a VICN:8:construction-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/construction-tool.hvif
		-a VICN:9:split-curves-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/split-curves-tool.hvif
		-a VICN:10:text-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/text-tool.hvif
		-a VICN:11:distance-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/distance-tool.hvif
		-a VICN:12:angle-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/angle-tool.hvif
		-a VICN:13:horizontal-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/horizontal-tool.hvif
		-a VICN:14:vertical-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/vertical-tool.hvif
		-a VICN:15:parallel-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/parallel-tool.hvif
		-a VICN:16:open-eye ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/open-eye.hvif
		-a VICN:17:closed-eye ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/closed-eye.hvif
		-a VICN:18:nearest-iso-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/nearest-iso-tool.hvif
		-a VICN:19:nearest-ortho-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/nearest-ortho-tool.hvif
		-a VICN:20:constrain-perpendicular-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/constrain-perpendicular-tool.hvif
		-a VICN:21:constrain-point-on-line-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/constrain-point-on-line-tool.hvif
		-a VICN:22:constrain-symmetric-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/constrain-symmetric-tool.hvif
		-a VICN:23:constrain-equal-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/constrain-equal-tool.hvif
		-a VICN:24:constrain-parallel-normals-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/constrain-parallel-normals-tool.hvif
		-a VICN:25:other-supplementary-angle-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/other-supplementary-angle-tool.hvif
		-a VICN:26:ref-tool ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/ref-tool.hvif
        COMMAND mimeset -f ${CMAKE_BINARY_DIR}/SolveSpace
	DEPENDS solvespace/Resources.rdef
)

# test suite
set(testsuite_sources
    test/analysis/contour_area/test.cpp
    test/core/expr/test.cpp
    test/core/locale/test.cpp
    test/core/path/test.cpp
    test/constraint/points_coincident/test.cpp
    test/constraint/pt_pt_distance/test.cpp
    test/constraint/pt_plane_distance/test.cpp
    test/constraint/pt_line_distance/test.cpp
    test/constraint/pt_face_distance/test.cpp
    test/constraint/proj_pt_distance/test.cpp
    test/constraint/pt_in_plane/test.cpp
    test/constraint/pt_on_line/test.cpp
    test/constraint/pt_on_face/test.cpp
    test/constraint/equal_length_lines/test.cpp
    test/constraint/length_ratio/test.cpp
    test/constraint/eq_len_pt_line_d/test.cpp
    test/constraint/eq_pt_ln_distances/test.cpp
    test/constraint/equal_angle/test.cpp
    test/constraint/equal_line_arc_len/test.cpp
    test/constraint/length_difference/test.cpp
    test/constraint/symmetric/test.cpp
    test/constraint/symmetric_horiz/test.cpp
    test/constraint/symmetric_vert/test.cpp
    test/constraint/symmetric_line/test.cpp
    test/constraint/at_midpoint/test.cpp
    test/constraint/horizontal/test.cpp
    test/constraint/vertical/test.cpp
    test/constraint/diameter/test.cpp
    test/constraint/pt_on_circle/test.cpp
    test/constraint/same_orientation/test.cpp
    test/constraint/angle/test.cpp # actually borken
    test/constraint/parallel/test.cpp
    test/constraint/perpendicular/test.cpp
    test/constraint/arc_line_tangent/test.cpp
    test/constraint/cubic_line_tangent/test.cpp
    test/constraint/curve_curve_tangent/test.cpp
    test/constraint/equal_radius/test.cpp
    test/constraint/where_dragged/test.cpp
    test/constraint/comment/test.cpp
    test/request/arc_of_circle/test.cpp
    test/request/circle/test.cpp
    test/request/cubic/test.cpp
    test/request/cubic_periodic/test.cpp
    test/request/datum_point/test.cpp
    test/request/image/test.cpp
    test/request/line_segment/test.cpp
    test/request/ttf_text/test.cpp
    test/request/workplane/test.cpp
    test/group/link/test.cpp
    test/group/translate_asy/test.cpp
    test/group/translate_nd/test.cpp
)

add_executable(solvespace-testsuite
    test/harness.cpp
	${testsuite_sources}
	src/platform/guinone.cpp
	$<TARGET_OBJECTS:solvespace-core>
	$<TARGET_PROPERTY:resources,EXTRA_SOURCES>)

target_link_libraries(solvespace-testsuite
	dxfrw agg ${ZLIB_LIBRARY} ${PNG_LIBRARY} ${FREETYPE_LIBRARY} ${COVERAGE_LIBRARY})

target_include_directories(solvespace-testsuite
	PRIVATE
	${EIGEN3_INCLUDE_DIRS}
	/boot/home/cppfront/include/
	/boot/system/develop/headers/agg2/
	src/
	test/)

add_dependencies(solvespace-testsuite resources)

add_custom_target(test_solvespace
    COMMAND $<TARGET_FILE:solvespace-testsuite>
    COMMENT "Testing SolveSpace"
    VERBATIM)

# debug runner
add_executable(solvespace-debugtool
    test/debugtool.cpp
	src/platform/guinone.cpp
	$<TARGET_OBJECTS:solvespace-core>
    $<TARGET_PROPERTY:resources>)

target_link_libraries(solvespace-debugtool solvespace-core dxfrw agg ${ZLIB_LIBRARY} ${PNG_LIBRARY} ${FREETYPE_LIBRARY})

target_include_directories(solvespace-debugtool PRIVATE
	/boot/home/cppfront/include/
	/boot/system/develop/headers/agg2/
	src/
)

add_dependencies(solvespace-debugtool resources)

# coverage reports
if(ENABLE_COVERAGE)
    find_program(GCOV gcov)
    find_program(LCOV lcov)
    find_program(GENHTML genhtml)

    set(LCOV_FLAGS   -q --gcov-tool ${GCOV})
    set(LCOV_FLAGS   ${LCOV_FLAGS} --rc lcov_branch_coverage=1)
    set(LCOV_FLAGS   ${LCOV_FLAGS} --rc "lcov_excl_line=(ssassert|switch)")
    set(LCOV_FLAGS   ${LCOV_FLAGS} --rc "lcov_excl_br_line=BRANCH_ALWAYS_TAKEN")
    set(LCOV_COLLECT -c -b ${CMAKE_SOURCE_DIR}/src --no-external -d ${CMAKE_CURRENT_BINARY_DIR}/)

    add_custom_command(
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/coverage_base.info
        COMMAND ${LCOV} ${LCOV_FLAGS} ${LCOV_COLLECT}
            -o ${CMAKE_BINARY_DIR}/coverage_base.info -i
        DEPENDS solvespace-testsuite
        COMMENT "Importing baseline coverage data"
        VERBATIM)

    add_custom_target(coverage_solvespace ALL
        COMMAND ${LCOV} ${LCOV_FLAGS} ${LCOV_COLLECT}
            -o ${CMAKE_BINARY_DIR}/coverage_test.info
        COMMAND ${LCOV} ${LCOV_FLAGS}
            -o ${CMAKE_BINARY_DIR}/coverage_full.info
            -a ${CMAKE_BINARY_DIR}/coverage_base.info
            -a ${CMAKE_BINARY_DIR}/coverage_test.info
        COMMAND ${LCOV} ${LCOV_FLAGS} --summary
            ${CMAKE_BINARY_DIR}/coverage_full.info
        COMMAND ${GENHTML} -q --branch-coverage --demangle-cpp --legend
            ${CMAKE_BINARY_DIR}/coverage_full.info
            -o ${CMAKE_BINARY_DIR}/coverage/
            -t "SolveSpace testbench"
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/coverage_base.info
        DEPENDS test_solvespace
        COMMENT "Generating coverage report"
        VERBATIM)
endif()