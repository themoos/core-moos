#this builds a test application which can be used
#to test and stress the MOOSDB
set(EXECNAME uDBTestClient)

#what files are needed?
SET(SRCS DBTestClient.cpp)

include_directories( ${${EXECNAME}_INCLUDE_DIRS} ${MOOS_INCLUDE_DIRS} ${MOOS_DEPEND_INCLUDE_DIRS})
add_executable(${EXECNAME} ${SRCS} )
target_link_libraries(${EXECNAME} ${MOOS_LIBRARIES} ${MOOS_DEPEND_LIBRARIES})


