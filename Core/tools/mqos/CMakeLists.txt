#this builds an application which can be used
#to monitor the QoS in moos

add_executable(mqos mqos.cpp)
target_link_libraries(mqos ${MOOS_LIBRARIES})

install(TARGETS mqos
  RUNTIME DESTINATION bin
)
