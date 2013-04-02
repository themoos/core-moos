

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests/lib)

include_directories( ${MOOS_INCLUDE_DIRS} ${MOOS_DEPEND_INCLUDE_DIRS})

add_executable(periodic_test PeriodicEventTest.cpp )
target_link_libraries(periodic_test ${MOOS_LIBRARIES} ${MOOS_DEPEND_LIBRARIES})

