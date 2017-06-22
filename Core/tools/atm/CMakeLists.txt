#this builds an application which can be used
#to audit MOOS comms

include_directories(${MOOS_INCLUDE_DIRS} ${MOOS_DEPEND_INCLUDE_DIRS})
add_executable(atm atm.cpp)
target_link_libraries(atm ${MOOS_LIBRARIES} ${MOOS_DEPEND_LIBRARIES})

install(TARGETS atm
  RUNTIME DESTINATION bin
)
