#this builds a test application which can be used
#to test and stress the MOOSDB
set(EXECNAME1 uDBTestClient)
set(EXECNAME2 uDBAsyncTest)

#what files are needed?
SET(SRCS1 DBTestClient.cpp)
SET(SRCS2 DBAsyncTest.cpp)

include_directories( ${${EXECNAME1}_INCLUDE_DIRS} ${MOOS_INCLUDE_DIRS} ${MOOS_DEPEND_INCLUDE_DIRS})
add_executable(${EXECNAME1} ${SRCS1} )
target_link_libraries(${EXECNAME1} ${MOOS_LIBRARIES} ${MOOS_DEPEND_LIBRARIES})

include_directories( ${${EXECNAME2}_INCLUDE_DIRS} ${MOOS_INCLUDE_DIRS} ${MOOS_DEPEND_INCLUDE_DIRS})
add_executable(${EXECNAME2} ${SRCS2} )
target_link_libraries(${EXECNAME2} ${MOOS_LIBRARIES} ${MOOS_DEPEND_LIBRARIES})

