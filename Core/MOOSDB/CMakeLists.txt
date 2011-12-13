set(EXECNAME MOOSDB)

# next few lines are simply to insert icons into
# the console application on windows
IF(WIN32)
    SET(RESOURCES icon1.ico MOOSDB.rc)
ENDIF(WIN32)

#what files are needed?
SET(SRCS MOOSDB.cpp MOOSDBMain.cpp MOOSDBVar.cpp MOOSRegisterInfo.cpp
    MOOSDB.h MOOSDBVar.h MOOSRegisterInfo.h MOOSDBHTTPServer.cpp MOOSDBHTTPServer.h HTTPConnection.cpp HTTPConnection.h
    ${RESOURCES})

include_directories( ${${EXECNAME}_INCLUDE_DIRS} ${MOOS_INCLUDE_DIRS} ${MOOS_DEPEND_INCLUDE_DIRS})
add_executable(${EXECNAME} ${SRCS} )
target_link_libraries(${EXECNAME} ${MOOS_LIBRARIES} ${MOOS_DEPEND_LIBRARIES})


add_subdirectory(testing)