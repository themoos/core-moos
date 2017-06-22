
if(UNIX)
    #this is a new addition allowing added for gcc to build
    #shared libraries against the MOOS libs..
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

add_subdirectory(libMOOS)
add_subdirectory(tools)
add_subdirectory(MOOSDB)
