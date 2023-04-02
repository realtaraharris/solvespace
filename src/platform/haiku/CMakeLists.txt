cmake_minimum_required (VERSION 3.20)

project(SolveSpace)

add_definitions('-g' '-Wall' '-Wextra' '-fpermissive' '-Wno-multichar' '-Wfatal-errors')
#add_definitions('-Wall' '-Wextra' '-fpermissive' '-Wno-multichar' '-Wfatal-errors')

include_directories("/boot/system/develop/headers/agg2/")
include_directories(${CMAKE_SOURCE_DIR})

include_directories("/system/develop/headers/gnu/")
include_directories("../../../src/")

link_directories("~/config/non-packaged/lib/")

add_executable(SolveSpace
	ui/App.cpp
	ui/MainWindow.cpp
#	ui/Solver.cpp

	ui/Toolbar.cpp
	ui/HVIFUtil.cpp
	ui/DrawButton.cpp

	ui/EditorView.cpp
)

set_property(TARGET SolveSpace PROPERTY CXX_STANDARD 11)

target_include_directories(SolveSpace
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${EIGEN3_INCLUDE_DIRS}
    {AGG_INCLUDE_DIRS}
    ../../src
)

target_link_libraries(SolveSpace be agg translation tracker solvespace-headless)

add_custom_command(TARGET SolveSpace POST_BUILD
	COMMAND rc -o ${CMAKE_BINARY_DIR}/Resources.rsrc ${CMAKE_SOURCE_DIR}/src/platform/haiku/Resources.rdef
	COMMAND xres -o ${CMAKE_BINARY_DIR}/bin/SolveSpace ${CMAKE_BINARY_DIR}/Resources.rsrc
		-a VICN:1:line-tool  ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/line-tool.hvif
		-a VICN:2:rect-tool  ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/rect-tool.hvif
		-a VICN:3:circle-tool  ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/circle-tool.hvif
		-a VICN:4:arc-tool  ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/arc-tool.hvif
		-a VICN:5:tangent-arc-tool  ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/tangent-arc-tool.hvif
		-a VICN:6:cubic-spline-tool  ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/cubic-spline-tool.hvif
		-a VICN:7:datum-point-tool  ${CMAKE_SOURCE_DIR}/src/platform/haiku/resources/icons/datum-point-tool.hvif
	COMMAND mimeset -f ${CMAKE_BINARY_DIR}/bin/SolveSpace
	DEPENDS solvespace/Resources.rdef
)