####################
# UltimateTD CMake #
####################

cmake_minimum_required( VERSION 2.6 )

project( UltimateTD )
SET ( BINSOURCES exceptions.h
				 handle_recycler.h
				 handle_name_allocator.h
				 streamable.h encoding.h
				 common.h common.cpp
				 prng.h prng.c
				 fismath.h fismath.cpp
				 atlas.h atlas.cpp
				 atlas_image_geom.h
				 atlas_image.h atlas_image.cpp
				 buf2.h buf2.cpp
				 tile.h tile.cpp
				 tile_builtin_sets.h tile_builtin_sets.cpp
				 map2.h map2.cpp
				 map_render.h map_render.cpp
				 widgets.h widgets.cpp
				 editor_widgets.h main.cpp )

##############
# Executable #
##############

add_executable( FUTD ${BINSOURCES} )
target_link_libraries( FUTD ${LIBRARIES} )

######################
# MSVC configuration #
######################
if(WIN32)
	if(MSVC)
	    set_target_properties( FUTD
							   PROPERTIES
								WIN32_EXECUTABLE true
								LINK_FLAGS_RELEASE -s
								DEBUG_POSTFIX "_d")
	else(MSVC)
		set_target_properties( FUTD
							   PROPERTIES
								LINK_FLAGS_RELEASE -s
								DEBUG_POSTFIX "_d")
	endif(MSVC)
endif(WIN32)
