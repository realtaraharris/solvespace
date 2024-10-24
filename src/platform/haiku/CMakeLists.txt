cmake_minimum_required (VERSION 3.20)

project(SolveSpace)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(-g -gdwarf-4 -Wno-deprecated-enum-float-conversion -Wno-conversion-null -Wno-deprecated-declarations -Wno-deprecated-enum-enum-conversion -Wno-register -std=c++20)

include_directories("/boot/system/develop/headers/agg2/")
include_directories(${CMAKE_SOURCE_DIR})

include_directories("/system/develop/headers/gnu/")
include_directories("../../../src/")

link_directories("~/config/non-packaged/lib/")
link_directories("../../../build/bin/")

add_executable(SolveSpace
	ui/App.cpp
	ui/MainWindow.cpp

	ui/Toolbar.cpp
	ui/PropertyBrowser.cpp
	ui/ListView/DragSortableListView.cpp
	ui/ListView/ThumbListItem.cpp
	ui/GroupsListView.cpp

	ui/HVIFUtil.cpp

	ui/EditorView.cpp
    ui/ViewParameters.cpp
    ../guihaiku.cpp

    ui/HaikuSpaceUI.cpp
)

# can't just set this because it'll emit -std=gnu++20 with no way to stop it :grimace:
#set_property(TARGET SolveSpace PROPERTY CXX_STANDARD 20)

target_include_directories(SolveSpace
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${EIGEN3_INCLUDE_DIRS}
    {AGG_INCLUDE_DIRS}
    ../../src
)

target_link_libraries(SolveSpace be agg translation tracker solvespace-headless)

add_custom_command(TARGET SolveSpace POST_BUILD
	COMMENT "rebuilding icon resources"
	COMMAND rc -o ${CMAKE_BINARY_DIR}/Resources.rsrc ${CMAKE_SOURCE_DIR}/src/platform/haiku/Resources.rdef
	COMMAND xres -o ${CMAKE_BINARY_DIR}/bin/SolveSpace ${CMAKE_BINARY_DIR}/Resources.rsrc
		-a VICN:1:line-tool  ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/line-tool.hvif
		-a VICN:2:rect-tool  ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/rect-tool.hvif
		-a VICN:3:circle-tool  ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/circle-tool.hvif
		-a VICN:4:arc-tool  ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/arc-tool.hvif
		-a VICN:5:tangent-arc-tool  ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/tangent-arc-tool.hvif
		-a VICN:6:cubic-spline-tool  ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/cubic-spline-tool.hvif
		-a VICN:7:datum-point-tool  ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/datum-point-tool.hvif
		-a VICN:8:construction-tool  ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/construction-tool.hvif
		-a VICN:9:split-curves-tool  ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/split-curves-tool.hvif
		-a VICN:10:text-tool  ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/text-tool.hvif
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
        COMMAND mimeset -f ${CMAKE_BINARY_DIR}/bin/SolveSpace
	DEPENDS solvespace/Resources.rdef
)
