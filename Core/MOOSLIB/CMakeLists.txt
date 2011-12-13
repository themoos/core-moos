set(LIBNAME MOOS)


#do we want to turn on the experimental clock skew detection?
OPTION(DETECT_CLOCK_DRIFT  "MOOSCommClient<->MOOSDB Clock drift detection (experimental)" OFF)
IF (DETECT_CLOCK_DRIFT)
    ADD_DEFINITIONS(-DMOOS_DETECT_CLOCK_DRIFT)
ENDIF (DETECT_CLOCK_DRIFT)
MARK_AS_ADVANCED(DETECT_CLOCK_DRIFT)

# Removed the extra field that was added to the MOOS message protocol in October 2009.
OPTION(DISABLE_AUX_SOURCE "Disable the AUX_SRC field in MOOS messages.  Only change this if you know what it is!" OFF)
IF (DISABLE_AUX_SOURCE)
  ADD_DEFINITIONS(-DDISABLE_AUX_SOURCE)
ENDIF (DISABLE_AUX_SOURCE)
MARK_AS_ADVANCED(DISABLE_AUX_SOURCE)

SET(SOURCES
MOOSVariable.cpp
MOOSCommClient.cpp
MOOSCommObject.cpp
MOOSCommPkt.cpp
MOOSCommServer.cpp
ThreadedCommServer.cpp
MOOSException.cpp
MOOSGlobalHelper.cpp
MOOSMsg.cpp
MOOSApp.cpp
MOOSInstrument.cpp
MOOSSkewFilter.cpp
XPCGetHostInfo.cpp
XPCGetProtocol.cpp
XPCSocket.cpp
XPCTcpSocket.cpp
XPCUdpSocket.cpp
)

##########################
# Packages / Dependencies

set(${LIBNAME}_DEPEND_INCLUDE_DIRS
    ${MOOS_SOURCE_DIR}/Core
    ${MOOSGen_INCLUDE_DIRS}
    ${PocoBits_INCLUDE_DIRS}
    CACHE INTERNAL
    "List of include dirs for the dependencies of ${LIBNAME}"    
)
list(REMOVE_DUPLICATES ${LIBNAME}_DEPEND_INCLUDE_DIRS)

set(${LIBNAME}_DEPEND_LIBRARIES
    ${MOOSGen_LIBRARIES}
    ${PocoBits_LIBRARIES}
    CACHE INTERNAL
    "List of libraries which are dependencies of ${LIBNAME}"    
)
list(REMOVE_DUPLICATES ${LIBNAME}_DEPEND_LIBRARIES)


#######################################
# Export include and library locations
# This is essentially boilerplate.  Shouldn't need to change it.

set(${LIBNAME}_INCLUDE_DIRS
    #${CMAKE_CURRENT_SOURCE_DIR}/include
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

MESSAGE(STATUS "MOOSLIB include path: ${${LIBNAME}_DEPEND_INCLUDE_DIRS}")


#####################
# Create the library
# This is boilerplate.  Any extra libs you want to link should be in the '..._DEPEND_...'
# variables
include_directories(${${LIBNAME}_INCLUDE_DIRS} ${${LIBNAME}_DEPEND_INCLUDE_DIRS})
add_library(${LIBNAME} ${SOURCES} ${PUBLIC_HEADERS})
target_link_libraries(${LIBNAME} ${${LIBNAME}_DEPEND_LIBRARIES})

#####################
# install headers
FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
INSTALL( FILES ${headers} DESTINATION include/MOOS/${LIBNAME} )

# install libraries
INSTALL(TARGETS ${LIBNAME} DESTINATION lib)

