set(LIBNAME MOOS)

set(COMMS_SOURCES
    Comms/MOOSVariable.cpp
    Comms/MOOSCommClient.cpp
    Comms/MOOSAsyncCommClient.cpp
    Comms/ClientCommsStatus.cpp
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
    Comms/ServerAudit.cpp
    Comms/ActiveMailQueue.cpp
    Comms/MessageQueueAccumulator.cpp
    Comms/SuicidalSleeper.cpp
    Comms/MulticastNode.cpp
    Comms/EndToEndAudit.cpp
)

set(APP_SOURCES
    App/MOOSApp.cpp
    App/MOOSInstrument.cpp
)


set(UTILS_SOURCES
    Utils/MOOSException.cpp
    Utils/MOOSLock.cpp
    Utils/MOOSFileReader.cpp
    Utils/MOOSUtilityFunctions.cpp
    Utils/ProcessConfigReader.cpp
    Utils/MOOSLinuxSerialPort.cpp
    Utils/MOOSSerialPort.cpp
    #  Utils/MOOSPlaybackStatus.cpp
    Utils/IPV4Address.cpp
    Utils/KeyboardCapture.cpp
    Utils/ThreadPrint.cpp
    Utils/CommandLineParser.cpp
    Utils/ProcInfo.cpp
    Utils/MemInfo.cpp
    Utils/ThreadPriority.cpp
    Utils/PeriodicEvent.cpp
    Utils/ConsoleColours.cpp
    Utils/CommsTools.cpp
)

if(WIN32)
    set(UTILS_SOURCES
        ${UTILS_SOURCES}
        Utils/NTSerial.cpp
        Utils/MOOSNTSerialPort.cpp
    )
endif()



set(THIRDPARTY_SOURCES
  Thirdparty/PocoBits/src/Event.cpp
  Thirdparty/PocoBits/src/Mutex.cpp
  Thirdparty/PocoBits/src/Timestamp.cpp
  Thirdparty/PocoBits/src/Exception.cpp
  Thirdparty/PocoBits/src/AtomicCounter.cpp
  Thirdparty/PocoBits/src/Bugcheck.cpp
  Thirdparty/PocoBits/src/Debugger.cpp
  Thirdparty/AppCasting/AppCastingMOOSApp.cpp
  Thirdparty/AppCasting/AppCastingMOOSInstrument.cpp
  Thirdparty/AppCasting/AppCast.cpp
)


set(DB_SOURCES
    DB/MOOSDB.cpp
    DB/MOOSDBVar.cpp
    DB/MOOSRegisterInfo.cpp
    DB/MsgFilter.cpp
    DB/HTTPConnection.cpp
    DB/MOOSDBHTTPServer.cpp
    DB/MOOSDBLogger.cpp
)



set(SOURCES ${THIRDPARTY_SOURCES} ${UTILS_SOURCES} ${COMMS_SOURCES} ${APP_SOURCES} ${DB_SOURCES})

include(PlatformDefines)

##############   FAST COMMS?   ################
#do we want to use the new fast asynchronous client architecture?
option(USE_ASYNC_COMMS  "enable fast asynchronous comms architecture" ON)
if(USE_ASYNC_COMMS)
  set(ASYNCHRONOUS_CLIENT_DEFINE "#define ASYNCHRONOUS_CLIENT")
endif()

#do we want to use the new fast asynchronous client architecture?
option(MOOS_DISABLE_XPCTCP_NAME_LOOKUP "Disable host name look ups" OFF)
if(MOOS_DISABLE_XPCTCP_NAME_LOOKUP)
    add_definitions(-DMOOS_DISABLE_XPCTCP_NAME_LOOKUP)
endif()


#do we want to enable multicast performance reporting
option(ENABLE_DETAILED_TIMING_AUDIT "enable detailed timing audit (for use with atm)" OFF)
if(ENABLE_DETAILED_TIMING_AUDIT)
    add_definitions(-DENABLE_DETAILED_TIMING_AUDIT)
endif()

#do we want to enable multicast performance reporting
option(DEFAULT_NO_NAGLE "by default turn off nagle (makes for lower latency)" ON)
if(DEFAULT_NO_NAGLE)
    add_definitions(-DDEFAULT_NO_NAGLE)
endif()



#do we want to turn on the experimental clock skew detection?
option(DETECT_CLOCK_DRIFT  "MOOSCommClient<->MOOSDB Clock drift detection (experimental)" OFF)
if(DETECT_CLOCK_DRIFT)
    add_definitions(-DMOOS_DETECT_CLOCK_DRIFT)
endif()
mark_as_advanced(DETECT_CLOCK_DRIFT)

# Remove the extra field that was added to the MOOS message protocol in October 2009.
option(DISABLE_AUX_SOURCE "Disable the AUX_SRC field in MOOS messages.  Only change this if you know what it is!" OFF)
if(DISABLE_AUX_SOURCE)
  add_definitions(-DDISABLE_AUX_SOURCE)
endif()
mark_as_advanced(DISABLE_AUX_SOURCE)


set(TIME_WARP_AGGLOMERATION_CONSTANT  0.0 CACHE STRING "comms agglomeration scale factor for high time warps")
if(${TIME_WARP_AGGLOMERATION_CONSTANT} GREATER 0.0)
  add_definitions(-DTIME_WARP_AGGLOMERATION_CONSTANT=${TIME_WARP_AGGLOMERATION_CONSTANT})
endif()
mark_as_advanced(TIME_WARP_AGGLOMERATION_CONSTANT)



option(ENABLE_V10_COMPATIBILITY  "Enable compatibility for source which includes pre V10 headers " OFF)
option(ENABLE_V10_IVP_COMPATIBILITY  "Enable compatibility for source which includes pre V10 headers as expected by IvP " OFF)
mark_as_advanced(ENABLE_V10_IVP_COMPATIBILITY)

if(ENABLE_V10_COMPATIBILITY)
    if(ENABLE_V10_IVP_COMPATIBILITY)
        set(COMPATIBILITY_HEADER_ROOTS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MOOS/Compatibility/Core/MOOSLIB
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MOOS/Compatibility/Core/MOOSGenLib
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MOOS/Compatibility/Essentials/MOOSUtlityLib
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MOOS/Compatibility/Core
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MOOS/Compatibility/Essentials)
        message(WARNING "IVP V10 compatibility will be removed soon - please consider adding qualified include directories")
    else()
        set(COMPATIBILITY_HEADER_ROOTS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MOOS/Compatibility/Core
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MOOS/Compatibility/Essentials)
    endif()
endif()

configure_file(${CMAKE_MODULE_PATH}/ClientDefines.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/App/include/MOOS/libMOOS/App/ClientDefines.h
    @ONLY
)


##########################
# Packages / Dependencies
set(INCLUDE_ROOTS

# this first entry is most likely empty but some souls may not want to update headers.
# placing this location at the start of the search list will enable some
# indirection

    ${COMPATIBILITY_HEADER_ROOTS}

# these are standard locations for the (functional) partitioning of the libMOOS
# library

    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/App/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Comms/include
    ${CMAKE_CURRENT_SOURCE_DIR}/DB/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Utils/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/PocoBits/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/getpot/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Thirdparty/AppCasting/include
)

set(${LIBNAME}_DEPEND_INCLUDE_DIRS
    ${INCLUDE_ROOTS}
    CACHE INTERNAL
    "List of include dirs for the dependencies of ${LIBNAME}"
)
list(REMOVE_DUPLICATES ${LIBNAME}_DEPEND_INCLUDE_DIRS)

if(UNIX)
    set(DEPENDENCIES pthread m)
endif()

if(WIN32)
  set(DEPENDENCIES wsock32 comctl32 ws2_32) # adding ws2_32 to support IPv6
  cache_internal_append_unique(PROJECT_EXPORT_DEFINES _WIN32_WINNT=0x600) # XP does not support IPv6. Moving to Vista and latter.
  add_definitions(-D_WIN32_WINNT=0x600)
endif()


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
    ${INCLUDE_ROOTS}
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


######################################
#export the library - this is picked up and used by the chunk of
#script at the end of the top level CMakeLists.txt file
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
install(DIRECTORY ${INCLUDE_ROOTS} DESTINATION ${CMAKE_INSTALL_PREFIX} FILES_MATCHING PATTERN "*.h" PATTERN "*.hxx")


# install libraries
install(TARGETS ${LIBNAME}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)


add_subdirectory(testing)

