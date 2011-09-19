PROJECT(MOOSGenLib)

set(LIBNAME MOOSGenLib)

SET(SOURCES
   MOOSLock.cpp
   MOOSFileReader.cpp
   MOOSGenLibGlobalHelper.cpp
   ProcessConfigReader.cpp
   MOOSLinuxSerialPort.cpp
   MOOSSerialPort.cpp
   MOOSLock.h
   MOOSFileReader.h
   MOOSGenLibGlobalHelper.h
   ProcessConfigReader.h
   MOOSLinuxSerialPort.h
   MOOSSerialPort.h
   MOOSThread.h
   MOOSSafeList.h
   MOOSScopedLock.h
   )

IF(WIN32)
    SET(SOURCES ${SOURCES} NTSerial.cpp MOOSNTSerialPort.cpp NTSerial.h MOOSNTSerialPort.h )
ENDIF(WIN32)


SET(ALLOW_RELEASE_ASSERTS FALSE CACHE BOOL "If TRUE then MOOSAssert is active in Release builds")

SET(MOOS_ASSERT_LEVEL "SOFT" CACHE STRING "OFF, SOFT (default) or HARD.  OFF: No asserts tested, SOFT: Prints error message but attempts to continue, HARD: Prints error message and exits.")
IF (MOOS_ASSERT_LEVEL MATCHES "OFF")
	ADD_DEFINITIONS(-DMOOS_ASSERT_LEVEL=0)
ELSEIF (MOOS_ASSERT_LEVEL MATCHES "SOFT")
	ADD_DEFINITIONS(-DMOOS_ASSERT_LEVEL=1)
ELSEIF (MOOS_ASSERT_LEVEL MATCHES "HARD")
	ADD_DEFINITIONS(-DMOOS_ASSERT_LEVEL=2)
ELSE (MOOS_ASSERT_LEVEL MATCHES "OFF")
	# Default
	ADD_DEFINITIONS(-DMOOS_ASSERT_LEVEL=1)
ENDIF (MOOS_ASSERT_LEVEL MATCHES "OFF")


##########################
# Packages / Dependencies

set(${LIBNAME}_DEPEND_INCLUDE_DIRS
   ""
    CACHE INTERNAL
    "List of include dirs for the dependencies of ${LIBNAME}"    
)
list(REMOVE_DUPLICATES ${LIBNAME}_DEPEND_INCLUDE_DIRS)

set(${LIBNAME}_DEPEND_LIBRARIES
    ""
    CACHE INTERNAL
    "List of libraries which are dependencies of ${LIBNAME}"    
)
list(REMOVE_DUPLICATES ${LIBNAME}_DEPEND_LIBRARIES)

#######################################
# Export include and library locations
# This is essentially boilerplate.  Shouldn't need to change it.

set(${LIBNAME}_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE INTERNAL
    "Where to find ${LIBNAME} include directories"
)
list(REMOVE_DUPLICATES ${LIBNAME}_INCLUDE_DIRS)

set(${LIBNAME}_LIBRARIES
    ${LIBNAME}
    CACHE INTERNAL
    "List of libraries needed to link ${LIBNAME} library"
)
list(REMOVE_DUPLICATES ${LIBNAME}_LIBRARIES)



#####################
# Create the library
# This is boilerplate.  Any extra libs you want to link should be in the '..._DEPEND_...'
# variables
include_directories(${${LIBNAME}_INCLUDE_DIRS} ${${LIBNAME}_DEPEND_INCLUDE_DIRS})
add_library(${LIBNAME} ${SOURCES} ${PUBLIC_HEADERS})
target_link_libraries(${LIBNAME} ${${LIBNAME}_DEPEND_LIBRARIES})





#SET( INSTALL_HEADERS
#MOOSLock.h
#MOOSFileReader.h
#MOOSGenLib.h
#MOOSGenLibGlobalHelper.h
#ProcessConfigReader.h
#MOOSLinuxSerialPort.h
#MOOSSerialPort.h
#MOOSAssert.h
#MOOSThread.h )

# install headers
#INSTALL( FILES ${INSTALL_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/MOOSGenLib )

# install libraries
#INSTALL(TARGETS
#MOOSGen
#RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
#LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
#ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
#)

