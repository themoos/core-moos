#this builds an application which can be used
#to graph MOOS connectivity

add_executable(ptm ptm.cpp)
target_link_libraries(ptm MOOS)

install(TARGETS ptm
  RUNTIME DESTINATION bin
)
