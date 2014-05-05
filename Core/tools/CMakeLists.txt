
foreach(DEFINE ${PROJECT_EXPORT_DEFINES})        
    ADD_DEFINITIONS(-D${DEFINE})
    IF ($ENV{VERBOSE})        
      message(STATUS "adding -D${DEFINE} as required by MOOS")
    ENDIF()
endforeach()


add_subdirectory(umm)
