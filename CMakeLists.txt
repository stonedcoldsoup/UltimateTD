####################
# UltimateTD CMake #
####################

# This file was created by modifying the
# CMakeLists.txt file from PhoenixCore, as
# it has all of the dependencies of PhoenixCore.

CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

PROJECT( UltimateTD )

set(CMAKE_CXX_FLAGS_RELEASE "-Wall -std=c++0x -O3 -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -std=c++0x -g -fno-strict-aliasing")

set(CMAKE_C_FLAGS_RELEASE "-Wall -ansi -O3 -fno-strict-aliasing")
set(CMAKE_C_FLAGS_DEBUG "-Wall -ansi -g -fno-strict-aliasing")

#############################
# Disallow in-source builds #
#############################

IF( NOT UNIX )
    IF( "${UltimateTD_SOURCE_DIR}" STREQUAL "${UltimateTD_BINARY_DIR}" )
	    MESSAGE( FATAL_ERROR "\nUltimateTD must NOT be built in source.\n"
			    "Run cmake in a child directory of the project.\n" )
    ENDIF ( "${UltimateTD_SOURCE_DIR}" STREQUAL "${UltimateTD_BINARY_DIR}" )
ENDIF ( NOT UNIX )

######################
# Check Dependencies #
######################

MESSAGE ( "Searching for dependencies..." )


FIND_PACKAGE( Threads REQUIRED )
SET( LIBRARIES ${LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )

set(Boost_USE_STATIC_LIBS	 ON)
set(Boost_USE_MULTITHREADED	 ON)
FIND_PACKAGE( Boost 1.39 REQUIRED COMPONENTS date_time system thread )
IF( Boost_FOUND )
    MESSAGE( " Boost found." )
else( Boost_FOUND )
    MESSAGE( FATAL_ERROR " Missing dependency: Boost 1.39 or higher." )
endif( Boost_FOUND )
include_directories( ${Boost_INCLUDE_DIRS} )
SET( LIBRARIES ${LIBRARIES} ${Boost_LIBRARIES} )
link_directories ( ${Boost_LIBRARY_DIRS} )

FIND_PACKAGE( OpenGL REQUIRED )
if( OPENGL_FOUND )
    MESSAGE( " OpenGL found." )
else(OPENGL_FOUND)
    MESSAGE( FATAL_ERROR " Missing dependency: OpenGL." )
endif(OPENGL_FOUND)
SET( INCLUDES ${INCLUDES} ${OPENGL_INCLUDE_DIR} )
SET( LIBRARIES ${LIBRARIES} ${OPENGL_LIBRARIES} )

# FIND_PACKAGE( GLU REQUIRED )
# if( GLU_FOUND )
#     MESSAGE( " GLU found." )
# else(GLU_FOUND)
#     MESSAGE( FATAL_ERROR " Missing dependency: GLU." )
# endif(GLU_FOUND)
# SET( INCLUDES ${INCLUDES} ${GLU_INCLUDE_DIR} )
# SET( LIBRARIES ${LIBRARIES} ${GLU_LIBRARIES} )

#############
# Find GLFW #
#############

set( SEARCH_PATHS
	${GLFW_PATH}
	${GLFW_PATH}/include
	${GLFW_PATH}/lib
	${GLFW_PATH}/lib/win32/Release # 2.7-lite
	${GLFW_PATH}/lib/win32/Debug
	${GLFW_PATH}/support/msvc80/Release # 2.6 Window Support
	${GLFW_PATH}/support/msvc80/Debug
	)

FIND_PATH( GLFW_INCLUDE_DIR GL/glfw.h PATHS ${SEARCH_PATHS} )
FIND_LIBRARY( GLFW_LIBRARY glfw PATHS ${SEARCH_PATHS} )

IF( GLFW_INCLUDE_DIR AND GLFW_LIBRARY )
	SET( GLFW_FOUND TRUE )
ENDIF( GLFW_INCLUDE_DIR AND GLFW_LIBRARY )

IF( GLFW_FOUND )
	MESSAGE (" GLFW found.")
ELSE( GLFW_FOUND )
	MESSAGE( FATAL_ERROR " Missing dependency: GLFW.\n"
						 " Please provide the root path\n"
						 " of your GLFW installation.\n"
						 " -DGLFW_PATH=\"path\"" )
ENDIF( GLFW_FOUND )

SET( INCLUDES ${INCLUDES} ${GLFW_INCLUDE_DIR} )

## On Windows GLFW is more likely to be static, so we place it
## after the system libraries to make sure it links properly.
if(WIN32)
	SET( LIBRARIES ${GLFW_LIBRARY} ${LIBRARIES} )
else(WIN32)
	SET( LIBRARIES ${LIBRARIES} ${GLFW_LIBRARY} )
endif(WIN32)

## GLFW Window Move Hack.

if( _GLFW_WM_MOVE_HACK )
    set_property( DIRECTORY
	PROPERTY COMPILE_DEFINITIONS _GLFW_WM_MOVE_HACK
    )
endif()

####################
# Find PhoenixCore #
####################

set( SEARCH_PATHS
	${PHOENIXCORE_PATH}
	${PHOENIXCORE_PATH}/source
	${PHOENIXCORE_PATH}/../source
	${PHOENIXCORE_PATH}/build/lib
	${PHOENIXCORE_PATH}/lib
	)

FIND_PATH( PHOENIXCORE_INCLUDE_DIR Phoenix.h PATHS ${SEARCH_PATHS} )
FIND_LIBRARY( PHOENIXCORE_LIBRARY PhoenixCore PATHS ${SEARCH_PATHS} )

IF( PHOENIXCORE_INCLUDE_DIR AND PHOENIXCORE_LIBRARY )
	SET( PHOENIXCORE_FOUND TRUE )
ENDIF( PHOENIXCORE_INCLUDE_DIR AND PHOENIXCORE_LIBRARY )

IF( PHOENIXCORE_FOUND )
	MESSAGE (" PhoenixCore found.")
ELSE( PHOENIXCORE_FOUND )
	MESSAGE( FATAL_ERROR " Missing dependency: PhoenixCore.\n"
						 " Please provide the root path\n"
						 " of your PhoenixCore installation.\n"
						 " -DPHOENIXCORE_PATH=\"path\"" )
ENDIF( PHOENIXCORE_FOUND )

SET( INCLUDES ${INCLUDES} ${PHOENIXCORE_INCLUDE_DIR} )
SET( LIBRARIES ${PHOENIXCORE_LIBRARY} ${LIBRARIES} )

set( SEARCH_PATHS
	${FUNGUSBOOSTER_PATH}
	${FUNGUSBOOSTER_PATH}/lib
	)

FIND_PATH( FUNGUSBOOSTER_INCLUDE_DIR fungus_booster/fungus_booster.h PATHS ${SEARCH_PATHS} )

if (DLL_FUNGUSUTIL)
	if (CMAKE_BUILD_TYPE MATCHES Debug)
		FIND_LIBRARY( FUNGUSBOOSTER_LIBRARY fungus_booster_d.dll PATHS ${SEARCH_PATHS} )
	else(CMAKE_BUILD_TYPE MATCHES Debug)
		FIND_LIBRARY( FUNGUSBOOSTER_LIBRARY fungus_booster.dll PATHS ${SEARCH_PATHS} )
	endif(CMAKE_BUILD_TYPE MATCHES Debug)
	if (FUNGUSBOOSTER_LIBRARY)
		message("Copying fungus_booster runtime...")
		file(COPY ${FUNGUSBOOSTER_PATH}/bin DESTINATION ${CMAKE_BINARY_DIR})
	else (FUNGUSBOOSTER_LIBRARY)
		message( FATAL_ERROR "not found!\n"
							 "Try building without -DDLL_FUNGUSUTIL.\n" )
	endif()
else (DLL_FUNGUSUTIL)
	if (CMAKE_BUILD_TYPE MATCHES Debug)
		FIND_LIBRARY( FUNGUSBOOSTER_LIBRARY fungus_booster_d PATHS ${SEARCH_PATHS} )
	else(CMAKE_BUILD_TYPE MATCHES Debug)
		FIND_LIBRARY( FUNGUSBOOSTER_LIBRARY fungus_booster PATHS ${SEARCH_PATHS} )
	endif(CMAKE_BUILD_TYPE MATCHES Debug)
	if (NOT FUNGUSBOOSTER_LIBRARY)
		message( FATAL_ERROR "Could not find fungus_booster library.\n"
							 "Try building with -DDLL_FUNGUSUTIL.\n" )
	endif()
endif(DLL_FUNGUSUTIL)

SET( LIBRARIES ${FUNGUSBOOSTER_LIBRARY} ${LIBRARIES} )
SET( INCLUDES ${INCLUDES} ${FUNGUSBOOSTER_INCLUDE_DIR} )

##################
# Platform Stuff #
##################

#WINDOWS (MSVC)
if(WIN32)

	if(MSVC)
	    MESSAGE( "Note: Building for MSVC.\n" )
        ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS) # No stupid warning about fopen and sprintf
    endif()

	ADD_DEFINITIONS(-DWIN32 -DMSVC)
    ADD_DEFINITIONS( -DGLEW_STATIC )

endif(WIN32)


# Linux
if(UNIX)

	MESSAGE( "Searching for *NIX dependencies..." )

	FIND_LIBRARY( XRANDR_LIBRARY Xrandr )
	IF( XRANDR_LIBRARY )
		MESSAGE(" Xrandr found.")
	ELSE( XRANDR_LIBRARY)
		MESSAGE( FATAL_ERROR " Missing dependency: XRandR.\n"
							 "     (GLFW requires XRandR on linux.)\n" )
	ENDIF( XRANDR_LIBRARY )

	SET( LIBRARIES ${LIBRARIES} ${XRANDR_LIBRARY} )

endif(UNIX )

#################################
# Change the output directories #
#################################
#SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${UltimateTD_SOURCE_DIR}/lib)
#SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${UltimateTD_SOURCE_DIR}/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${UltimateTD_SOURCE_DIR}/bin)
SET(CMAKE_OUTPUT_OUTPUT_DIRECTORY ${UltimateTD_SOURCE_DIR}/bin)

#######################
# Include Directories #
#######################

include_directories( ${UltimateTD_SOURCE_DIR}/source ${INCLUDES})

###################
# Include subdirs #
###################

subdirs( source )
#subdirs( tests )
#subdirs( examples )
