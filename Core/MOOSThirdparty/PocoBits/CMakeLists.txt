set(LIBNAME PocoBits)

set(SOURCES
  src/Event.cpp
  src/Mutex.cpp
  src/Timestamp.cpp
  src/Exception.cpp
)

set(HEADER_DIR include/PocoBits)

set(PUBLIC_HEADERS
  ${HEADER_DIR}/Event.h
  ${HEADER_DIR}/Mutex.h
  ${HEADER_DIR}/ScopedLock.h
  ${HEADER_DIR}/Exception.h
)

##########################
# Packages / Dependencies

set(${LIBNAME}_DEPEND_INCLUDE_DIRS
   ""
    CACHE INTERNAL
    "List of include dirs for the dependencies of ${LIBNAME}"    
)
list(REMOVE_DUPLICATES ${LIBNAME}_DEPEND_INCLUDE_DIRS)

set(${LIBNAME}_DEPEND_LIBRARIES
    ""
    CACHE INTERNAL
    "List of libraries which are dependencies of ${LIBNAME}"    
)
list(REMOVE_DUPLICATES ${LIBNAME}_DEPEND_LIBRARIES)

#######################################
# Export include and library locations
# This is essentially boilerplate.  Shouldn't need to change it.

set(${LIBNAME}_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    CACHE INTERNAL
    "Where to find ${LIBNAME} include directories"
)
list(REMOVE_DUPLICATES ${LIBNAME}_INCLUDE_DIRS)

set(${LIBNAME}_LIBRARIES
    ${LIBNAME}
    CACHE INTERNAL
    "List of libraries needed to link ${LIBNAME} library"
)
list(REMOVE_DUPLICATES ${LIBNAME}_LIBRARIES)



#####################
# Create the library
# This is boilerplate.  Any extra libs you want to link should be in the '..._DEPEND_...'
# variables
include_directories(${${LIBNAME}_INCLUDE_DIRS} ${${LIBNAME}_DEPEND_INCLUDE_DIRS})
add_library(${LIBNAME} ${SOURCES} ${PUBLIC_HEADERS})
target_link_libraries(${LIBNAME} ${${LIBNAME}_DEPEND_LIBRARIES})


