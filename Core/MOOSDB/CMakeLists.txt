#PROJECT(MOOSDB)
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

#do we want a webserver compiled in?
SET(DB_HAS_HTTP FALSE CACHE BOOL "DB serves HTTP (experimental)")
IF (DB_HAS_HTTP)
    ADD_DEFINITIONS(-DMOOSDB_HAS_WEBSERVER)
ENDIF (DB_HAS_HTTP)

include_directories( ${${EXECNAME}_INCLUDE_DIRS} ${MOOSLib_INCLUDE_DIRS} ${MOOSLib_DEPEND_INCLUDE_DIRS})
add_executable(${EXECNAME} ${SRCS} )
target_link_libraries(${EXECNAME} ${MOOSLib_LIBRARIES} ${MOOSLib_DEPEND_LIBRARIES})


add_subdirectory(testing)