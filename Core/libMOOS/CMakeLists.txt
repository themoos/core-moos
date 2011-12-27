set(LIBNAME MOOS)

SET(COMMS_SOURCES
    Comms/MOOSVariable.cpp
    Comms/MOOSCommClient.cpp
    Comms/MOOSCommObject.cpp
    Comms/MOOSCommPkt.cpp
    Comms/MOOSCommServer.cpp
    Comms/ThreadedCommServer.cpp
    Comms/MOOSMsg.cpp
    Comms/MOOSSkewFilter.cpp
    Comms/XPCGetHostInfo.cpp
    Comms/XPCGetProtocol.cpp
    Comms/XPCSocket.cpp
    Comms/XPCTcpSocket.cpp
    Comms/XPCUdpSocket.cpp
)

SET(APP_SOURCES
    App/MOOSApp.cpp
    App/MOOSInstrument.cpp
)


SET(UTILS_SOURCES
   Utils/MOOSException.cpp
   Utils/MOOSLock.cpp
   Utils/MOOSFileReader.cpp
   Utils/MOOSUtilityFunctions.cpp
   Utils/ProcessConfigReader.cpp
   Utils/MOOSLinuxSerialPort.cpp
   Utils/MOOSSerialPort.cpp
   Utils/MOOSPlaybackStatus.cpp

   )

IF(WIN32)
    SET(UTIL_SOURCES ${UTILS_SOURCES} NTSerial.cpp MOOSNTSerialPort.cpp NTSerial.h MOOSNTSerialPort.h )
ENDIF(WIN32)



set(THIRDPARTY_SOURCES
  Thirdparty/PocoBits/src/Event.cpp
  Thirdparty/PocoBits/src/Mutex.cpp
  Thirdparty/PocoBits/src/Timestamp.cpp
  Thirdparty/PocoBits/src/Exception.cpp
)


set(SOURCES ${THIRDPARTY_SOURCES} ${UTILS_SOURCES} ${COMMS_SOURCES} ${APP_SOURCES} ) 

include(PlatformDefines)


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


##########################
# Packages / Dependencies
set(INCLUDE_ROOTS  
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/App/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Comms/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/PocoBits/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/getpot/include)
    
set(${LIBNAME}_DEPEND_INCLUDE_DIRS
    ${INCLUDE_ROOTS}
    CACHE INTERNAL
    "List of include dirs for the dependencies of ${LIBNAME}"    
)
list(REMOVE_DUPLICATES ${LIBNAME}_DEPEND_INCLUDE_DIRS)

if(UNIX)
    set(DEPENDENCIES pthread m)
endif(UNIX)
set(${LIBNAME}_DEPEND_LIBRARIES
    ${DEPENDENCIES}
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


######################################
#export the library
cache_internal_append_unique(PROJECT_EXPORT_LIBS 
    ${LIBNAME}
)



#######################################
# Create the library
# This is boilerplate.  Any extra libs you want to link should be in the '..._DEPEND_...'
# variables
include_directories(${${LIBNAME}_INCLUDE_DIRS} ${${LIBNAME}_DEPEND_INCLUDE_DIRS})
add_library(${LIBNAME} ${SOURCES} ${PUBLIC_HEADERS})
target_link_libraries(${LIBNAME} ${${LIBNAME}_DEPEND_LIBRARIES})

#######################################
# install headers
install(DIRECTORY ${INCLUDE_ROOTS} DESTINATION ${CMAKE_INSTALL_PREFIX} FILES_MATCHING PATTERN "*.h")


# install libraries
INSTALL(TARGETS ${LIBNAME} DESTINATION lib)



