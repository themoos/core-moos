
###########################################################
#   Top level CMakeLists.txt file for the MOOS Project
#   pnewman@robots.ox.ac.uk
###########################################################


CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(PROJECTNAME MOOS)
project(${PROJECTNAME})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)


set(CMakeScripts_MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_MODULE_PATH ${CMakeScripts_MODULE_DIR})


# Turn on all compiler warnings
include(EnableAllWarnings)

# Bring in helper functions for dealing with CACHE INTERNAL variables
include(CacheInternalHelpers)

# We want to create dynamic libraries
set(BUILD_SHARED_LIBS true)

# Enable Doxygen build with 'make doxygen'
option(ENABLE_DOXYGEN "Enable a 'make doc' target for Doxygen documentation")
if (ENABLE_DOXYGEN)
    include(UseDoxygen)
endif()




############# CORE ? ################
SET( BUILD_CORE_MOOS ON CACHE BOOL "Build Core MOOS")
IF(BUILD_CORE_MOOS)
    add_subdirectory(Core)
ENDIF(BUILD_CORE_MOOS)


############# COMPRESSED COMMS? ################
SET(COMPRESSED_MOOS_PROTOCOL FALSE CACHE BOOL "Compressed comms protocol (experimental)")

### PMN - this should not be here! ####
MESSAGE(STATUS "FIX ME ZLIB DETECTION IS TOO HIGH LEVEL")

IF(COMPRESSED_MOOS_PROTOCOL)
	FIND_PACKAGE(ZLIB QUIET)
	IF (ZLIB_FOUND)
		ADD_DEFINITIONS(-DZLIB_FOUND)
    		ADD_DEFINITIONS(-DCOMPRESSED_MOOS_PROTOCOL)
    
		MESSAGE(STATUS "Cmake FindZLIB: using ZLIB includes at: ${ZLIB_INCLUDE_DIR}")
    		MESSAGE(STATUS "Cmake FindZLIB: using ZLIB libraries: ${ZLIB_LIBRARIES}")

		SET(PLATFORM_LIBS   ${PLATFORM_LIBS} ${ZLIB_LIBRARIES} )
		INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})	
	
	ELSE(ZLIB_FOUND)
		SET(ZLIB_LIBRARIES "")
	ENDIF (ZLIB_FOUND)
ENDIF(COMPRESSED_MOOS_PROTOCOL)


############# ESSENTIAL APPS ? ################
SET( BUILD_ESSENTIAL_MOOS OFF CACHE BOOL "Build Essential MOOS Processes (like pLogger pAntler etc)")
IF(BUILD_ESSENTIAL_MOOS)
    #here we name all the things we want built as part of the Essential MOOS
    add_subdirectory(Essentials)
ENDIF(BUILD_ESSENTIAL_MOOS)

############# CONTROL AND NAV ? ################
SET( BUILD_CONTROL_AND_NAV OFF CACHE BOOL "Build control and navigation Apps iRemote pHelm pNav")
IF(BUILD_CONTROL_AND_NAV)
    #here we name all the things we want built under the navigation and control heading
    add_subdirectory(NavigationAndControl)
ENDIF(BUILD_CONTROL_AND_NAV)

############# TOOLS ? ################
SET( BUILD_TOOLS OFF CACHE BOOL "Build GUI tools, simulators and matlab access")
IF(BUILD_TOOLS)
    #here we tell CMake to descend into the tools directory. More
    #decisions will be made in there
    add_subdirectory(Tools)
ENDIF(BUILD_TOOLS)


############# INSTRUMENTS ? ################
SET( BUILD_INSTRUMENTS OFF CACHE BOOL "Build Instruments ")
IF(BUILD_INSTRUMENTS)
    add_subdirectory(Instruments)
ENDIF(BUILD_INSTRUMENTS)

############# EXAMPLES ? ################
SET( BUILD_EXAMPLES OFF CACHE BOOL "Build Documentation Examples ")
IF(BUILD_EXAMPLES)
    add_subdirectory(Docs/ProgrammingWithMOOS/code)
    add_subdirectory(Docs/Essentials/Antler/code)
ENDIF(BUILD_EXAMPLES)


#####################################################
# some definitions are always needed...
IF(WIN32)
    ADD_DEFINITIONS(-DWINDOWS_NT -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_DEPRECATE)
ENDIF(WIN32)
IF(UNIX)
    ADD_DEFINITIONS(-DUNIX)
	IF("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
		ADD_DEFINITIONS( -DPLATFORM_DARWIN )
		
		#some support for macport installed libraries
		SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /opt/local/lib)
		SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} /opt/local/include)
		MESSAGE(STATUS "Cmake library path ${CMAKE_LIBRARY_PATH}")

		SET( PLATFORM_DARWIN 1 )
	ENDIF("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
	IF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
		ADD_DEFINITIONS( -DPLATFORM_LINUX )
		SET( PLATFORM_LINUX 1 )
	ENDIF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
ENDIF(UNIX)


#####################################################
# some platform libs are always required...
IF(WIN32)
  SET(PLATFORM_LIBS   ${PLATFORM_LIBS} wsock32 comctl32)
ENDIF(WIN32)
IF(UNIX)
  SET(PLATFORM_LIBS   ${PLATFORM_LIBS} m pthread)

  IF("${CMAKE_SYSTEM_NAME}" MATCHES "SunOS")
	SET(PLATFORM_LIBS ${PLATFORM_LIBS} socket nsl rt)
  ENDIF("${CMAKE_SYSTEM_NAME}" MATCHES "SunOS")

ENDIF(UNIX)


#####################################################
# Toby Schneider's request for fPIC on amd64
IF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
 SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC"  )
ENDIF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")



#########################
# Config Script Creation
#########################

# Add your library's name to PROJECT_EXPORT_LIBS if you wish to have it exported
# (This is a CACHE INTERNAL variable)
if (PROJECT_EXPORT_LIBS)
    
    message(STATUS "PROJECT_EXPORT_LIBS = ${PROJECT_EXPORT_LIBS}")
    
    # Provide a cmake config script for the build directory
    export(TARGETS
        ${PROJECT_EXPORT_LIBS}
        FILE
        Use${PROJECTNAME}.cmake
    )

    set(CONFIG_FILE "${PROJECT_BINARY_DIR}/${PROJECTNAME}Config.cmake")
    configure_file( "cmake/ProjectConfig.cmake.in" ${CONFIG_FILE} @ONLY )
    
    # Append libary specific lines to the generated config script
    foreach(EXPORT_LIB ${PROJECT_EXPORT_LIBS})
        file(APPEND ${CONFIG_FILE} "\n# Export variables for ${EXPORT_LIB} Library\n")
        file(APPEND ${CONFIG_FILE} "set(${EXPORT_LIB}_LIBRARIES \"${${EXPORT_LIB}_LIBRARIES}\")\n")
        file(APPEND ${CONFIG_FILE} "set(${EXPORT_LIB}_INCLUDE_DIRS \"${${EXPORT_LIB}_INCLUDE_DIRS}\")\n")
        file(APPEND ${CONFIG_FILE} "set(${EXPORT_LIB}_DEPEND_LIBRARIES \"${${EXPORT_LIB}_DEPEND_LIBRARIES}\")\n")
        file(APPEND ${CONFIG_FILE} "set(${EXPORT_LIB}_DEPEND_INCLUDE_DIRS \"${${EXPORT_LIB}_DEPEND_INCLUDE_DIRS}\")\n")
    endforeach()

endif()


# Stick build location in the central CMake user package registry, so that it may be easily found
if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.7)
  export(PACKAGE ${PROJECTNAME})
endif()



