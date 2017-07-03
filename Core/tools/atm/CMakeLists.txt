#this builds an application which can be used
#to audit MOOS comms

add_executable(atm atm.cpp)
target_link_libraries(atm ${MOOS_LIBRARIES})

install(TARGETS atm
  RUNTIME DESTINATION bin
)
