# First, set up registration functions for the kinds of resources we handle.
set(resource_root ${CMAKE_CURRENT_SOURCE_DIR}/)
set(resource_list)
set(resource_names)
include(GNUInstallDirs)

set(app_resource_dir ${CMAKE_BINARY_DIR}/res)

function(add_resource name)
	set(source ${CMAKE_CURRENT_SOURCE_DIR}/${name})
	set(target ${app_resource_dir}/${name})
	set(resource_list "${resource_list};${target}" PARENT_SCOPE)

	get_filename_component(target_dir ${target} DIRECTORY)
	add_custom_command(
		OUTPUT ${target}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${target_dir}
		COMMAND ${CMAKE_COMMAND} -E copy ${source} ${target}
		COMMENT "Copying resource ${name}"
		DEPENDS ${source}
		VERBATIM)

	get_filename_component(name_dir ${name} DIRECTORY)
	install(FILES ${source}
		DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/solvespace/${name_dir})
endfunction()

function(add_resources)
    foreach(name ${ARGN})
        add_resource(${name})
        set(resource_list  "${resource_list}" PARENT_SCOPE)
        set(resource_names "${resource_names}" PARENT_SCOPE)
    endforeach()
endfunction()

# Second, register all resources.
add_resources(
    banner.txt
    fonts/unifont.hex.gz
    fonts/private/0-check-false.png
    fonts/private/1-check-true.png
    fonts/private/2-radio-false.png
    fonts/private/3-radio-true.png
    fonts/private/4-stipple-dot.png
    fonts/private/5-stipple-dash-long.png
    fonts/private/6-stipple-dash.png
    fonts/private/7-stipple-zigzag.png
    fonts/unicode.lff.gz
    fonts/BitstreamVeraSans-Roman-builtin.ttf
    shaders/imesh.frag
    shaders/imesh.vert
    shaders/imesh_point.frag
    shaders/imesh_point.vert
    shaders/imesh_tex.frag
    shaders/imesh_texa.frag
    shaders/imesh_tex.vert
    shaders/mesh.frag
    shaders/mesh.vert
    shaders/mesh_fill.frag
    shaders/mesh_fill.vert
    shaders/edge.frag
    shaders/edge.vert
    shaders/outline.vert)

# Third, distribute the resources.
add_custom_target(resources
    DEPENDS ${resource_list})
