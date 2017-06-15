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
    list(APPEND UTILS_SOURCES
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

#do we want to use the new fast asynchronous client architecture?
option(USE_ASYNC_COMMS  "enable fast asynchronous comms architecture" ON)
if(USE_ASYNC_COMMS)
  set(ASYNCHRONOUS_CLIENT_DEFINE "#define ASYNCHRONOUS_CLIENT")
endif()

# Configure step picks up the ASYNCHRONOUS_CLIENT_DEFINE variable
configure_file(${CMAKE_MODULE_PATH}/ClientDefines.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/App/include/MOOS/libMOOS/App/ClientDefines.h
    @ONLY
)



##########################
# Packages / Dependencies
set(INCLUDE_ROOTS

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

if(UNIX)
    set(DEPENDENCIES pthread m)
endif()

if(WIN32)
  set(DEPENDENCIES wsock32 comctl32 ws2_32) # adding ws2_32 to support IPv6
endif()


set(MOOS_DEPEND_LIBRARIES
    ${DEPENDENCIES}
    CACHE INTERNAL
    "List of libraries which are dependencies of MOOS"
)
list(REMOVE_DUPLICATES MOOS_DEPEND_LIBRARIES)


#######################################
# Export include and library locations

set(MOOS_INCLUDE_DIRS
    ${INCLUDE_ROOTS}
    CACHE INTERNAL
    "Where to find MOOS include directories"
)
list(REMOVE_DUPLICATES MOOS_INCLUDE_DIRS)

set(MOOS_LIBRARIES
    MOOS
    CACHE INTERNAL
    "List of libraries needed to link MOOS library"
)
list(REMOVE_DUPLICATES MOOS_LIBRARIES)

#######################################
# Create the library
add_library(MOOS
    ${THIRDPARTY_SOURCES}
    ${UTILS_SOURCES}
    ${COMMS_SOURCES}
    ${APP_SOURCES}
    ${DB_SOURCES}
)

target_include_directories(MOOS PUBLIC
    ${MOOS_INCLUDE_DIRS}
)

target_link_libraries(MOOS ${MOOS_DEPEND_LIBRARIES})

if(WIN32)
    # XP does not support IPv6. Therefore MOOS only supports Vista and later.
    target_compile_definitions(MOOS PUBLIC _WIN32_WINNT=0x600)
endif()

#do we want to disable host name lookups?
option(MOOS_DISABLE_XPCTCP_NAME_LOOKUP "Disable host name look ups" OFF)
if(MOOS_DISABLE_XPCTCP_NAME_LOOKUP)
    target_compile_definitions(MOOS PRIVATE MOOS_DISABLE_XPCTCP_NAME_LOOKUP)
endif()

#do we want to enable multicast performance reporting
option(ENABLE_DETAILED_TIMING_AUDIT "enable detailed timing audit (for use with atm)" OFF)
if(ENABLE_DETAILED_TIMING_AUDIT)
    target_compile_definitions(MOOS PRIVATE ENABLE_DETAILED_TIMING_AUDIT)
endif()

#do we want to enable multicast performance reporting
option(DEFAULT_NO_NAGLE "by default turn off nagle (makes for lower latency)" ON)
if(DEFAULT_NO_NAGLE)
    target_compile_definitions(MOOS PRIVATE DEFAULT_NO_NAGLE)
endif()

#do we want to turn on the experimental clock skew detection?
option(DETECT_CLOCK_DRIFT  "MOOSCommClient<->MOOSDB Clock drift detection (experimental)" OFF)
if(DETECT_CLOCK_DRIFT)
    target_compile_definitions(MOOS PRIVATE MOOS_DETECT_CLOCK_DRIFT)
endif()
mark_as_advanced(DETECT_CLOCK_DRIFT)

set(TIME_WARP_AGGLOMERATION_CONSTANT  0.0 CACHE STRING "comms agglomeration scale factor for high time warps")
if(${TIME_WARP_AGGLOMERATION_CONSTANT} GREATER 0.0)
  target_compile_definitions(MOOS PRIVATE
      TIME_WARP_AGGLOMERATION_CONSTANT=${TIME_WARP_AGGLOMERATION_CONSTANT}
  )
endif()
mark_as_advanced(TIME_WARP_AGGLOMERATION_CONSTANT)


#######################################
# install headers
install(DIRECTORY ${INCLUDE_ROOTS} DESTINATION ${CMAKE_INSTALL_PREFIX} FILES_MATCHING PATTERN "*.h" PATTERN "*.hxx")


# install libraries
install(TARGETS MOOS
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

######################################
#export the library - this is picked up and used by the chunk of
#script at the end of the top level CMakeLists.txt file
set_property(GLOBAL APPEND PROPERTY PROJECT_EXPORT_LIBS
    MOOS
)

add_subdirectory(testing)

