
###########################################################
#   Top level CMakeLists.txt file for the core-moos Project
#   pnewman@robots.ox.ac.uk
###########################################################

cmake_minimum_required(VERSION 2.8.12)

cmake_policy(SET CMP0003 NEW)
if(POLICY CMP0042)
  cmake_policy(SET CMP0042 NEW)
endif()

set(PROJECTNAME MOOS)

set(PACKAGE_VERSION_MAJOR 10)
set(PACKAGE_VERSION_MINOR 2)
set(PACKAGE_VERSION_PATCH 0)

set(PACKAGE_VERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH}")
message(STATUS "+++  This is MOOS version ${PACKAGE_VERSION} +++")

# If user specifies the build type, use theirs, otherwise use Release
if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "")
endif()


project(${PROJECTNAME})



if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
endif()
if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
endif()
if(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
endif()

set(CMakeScripts_MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_MODULE_PATH ${CMakeScripts_MODULE_DIR})

# Turn on all compiler warnings
include(EnableAllWarnings)

# Bring in helper functions for dealing with CACHE INTERNAL variables
include(CacheInternalHelpers)

# Figure out our git version
option(UPDATE_GIT_VERSION_INFO "update git version info in source tree" ON)
mark_as_advanced(UPDATE_GIT_VERSION_INFO)
if(UPDATE_GIT_VERSION_INFO)
    include(GitInfo)
endif()

# Do we want to create dynamic libraries?
option(BUILD_SHARED_LIBS "Build lib${PROJECTNAME} as a shared library" OFF)

# Enable Doxygen build with 'make doxygen'
option(ENABLE_DOXYGEN "Enable a 'make doc' target for Doxygen documentation")
if(ENABLE_DOXYGEN)
    include(UseDoxygen)
endif()

# Option to enable CDash testing
option(CDASH_SUPPORT "Turn on testing targets that upload results to CDash" OFF)

# Option for code coverage
include(CMakeDependentOption)
set(REQUIRE_CDASH_SUPPORT OFF)
cmake_dependent_option(CODE_COVERAGE
    "Build code coverage results, requires GCC compiler (forces Debug build)" OFF
    "NOT CMAKE_COMPILER_IS_GNUCXX" OFF)
if(CODE_COVERAGE)
    set(CMAKE_CXX_FLAGS_DEBUG
        "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)

    # Ensure that CDash targets are always enabled if coverage is enabled.
    if(NOT CDASH_SUPPORT)
        get_property(HELP_STRING CACHE CDASH_SUPPORT PROPERTY HELPSTRING)
        set(CDASH_SUPPORT ON CACHE BOOL "${HELP_STRING}" FORCE)
        message(STATUS "Enabling CDash targets as coverage has been enabled.")
    endif()
endif()

if( CDASH_SUPPORT )
  enable_testing()

  # The BUILDNAME var is picked up by the CTest script
  include(SystemUtilityFunctions)
  moos_make_informative_build_name(BUILDNAME)
  include( CTest )
endif()

#optionally try to compile with c++11 if it is available
option(USE_CXX11_IF_AVAILABLE "use c++11 if available" ON)
if(USE_CXX11_IF_AVAILABLE)
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
    if(COMPILER_SUPPORTS_CXX11)
      if(CMAKE_VERSION VERSION_EQUAL 3.1.0 OR CMAKE_VERSION VERSION_GREATER 3.1.0)
        set(CMAKE_CXX_STANDARD 11)
        set(CMAKE_CXX_EXTENSIONS OFF)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
      else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
      endif()
    else()
      message(STATUS "no c++11 compiler found -> not using c++11")
    endif()
endif()

# Option for explicitly linking against the GNU libstdc++ instead of libc++ on
# OSX Mavericks, required in order to use iMatlab as MATLAB still uses libstdc++
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # On OSX, get the version number.
  find_program(SW_VERS_PROGRAM sw_vers)
  mark_as_advanced(SW_VERS_PROGRAM)
  execute_process(COMMAND ${SW_VERS_PROGRAM} "-productVersion"
    OUTPUT_VARIABLE OSX_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  # Mavericks (10.9) and above use libc++ as the default.
  set(OSX_MAVERICKS_VERSION 10.9)

  if(OSX_VERSION VERSION_EQUAL OSX_MAVERICKS_VERSION OR
      OSX_VERSION VERSION_GREATER OSX_MAVERICKS_VERSION)
    option(USE_LIBSTDCPP
      "Use (legacy) GNU libstdc++ on OSX >= ${OSX_MAVERICKS_VERSION}, instead of default LLVM libc++ (required for iMatlab)" OFF)
    if(USE_LIBSTDCPP)
      message("Running OS X Mavericks (specific version: ${OSX_VERSION}) using "
        "legacy GNU libstdc++ instead of default LLVM libc++, disable "
        "USE_LIBSTDCPP option to revert to default LLVM libc++.")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
    else()
      # Remove any previous specification of libstdc++.
      message(STATUS "Using default LLVM libc++.")
      string(REPLACE "-stdlib=libstdc++" "" CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS}")
    endif()
  endif()
endif()


###########  SETTING UP PREPROCESSOR ################
include(PlatformDefines)

if(UNIX OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    # Cause shared libraries to be built with the -fPIC flag
    # This flag requires CMake 2.8.9 or greater.
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

add_subdirectory(Core)

#########################
# Config Script Creation
#########################

# Add your library's name to the PROJECT_EXPORT_LIBS global property if you wish
# to have it exported.
option(ENABLE_EXPORT
  "Cause CMake to record this build so other projects can find it " ON)
if(ENABLE_EXPORT)
  message(STATUS "location of this build will be exported, set ENABLE_EXPORT to OFF if you don't want this and delete ~/.cmake/projects/MOOS ")
endif()

get_property(PROJECT_EXPORT_LIBS GLOBAL PROPERTY PROJECT_EXPORT_LIBS)
if(PROJECT_EXPORT_LIBS)

    message(STATUS "PROJECT_EXPORT_LIBS = ${PROJECT_EXPORT_LIBS}")

    # Provide a cmake config script for the build directory
    export(TARGETS
        ${PROJECT_EXPORT_LIBS}
        FILE
        Use${PROJECTNAME}.cmake
    )

    set(CONFIG_FILE "${PROJECT_BINARY_DIR}/${PROJECTNAME}Config.cmake")
    configure_file( "cmake/ProjectConfig.cmake.in" ${CONFIG_FILE} @ONLY )

    # Append libary specific lines to the generated config script
    foreach(EXPORT_LIB ${PROJECT_EXPORT_LIBS})
        file(APPEND ${CONFIG_FILE} "\n# Export variables for ${EXPORT_LIB} Library\n")
        file(APPEND ${CONFIG_FILE} "set(${EXPORT_LIB}_LIBRARIES \"${${EXPORT_LIB}_LIBRARIES}\")\n")
        file(APPEND ${CONFIG_FILE} "set(${EXPORT_LIB}_INCLUDE_DIRS \"${${EXPORT_LIB}_INCLUDE_DIRS}\")\n")
        file(APPEND ${CONFIG_FILE} "set(${EXPORT_LIB}_DEPEND_LIBRARIES \"${${EXPORT_LIB}_DEPEND_LIBRARIES}\")\n")
        file(APPEND ${CONFIG_FILE} "set(${EXPORT_LIB}_DEPEND_INCLUDE_DIRS \"${${EXPORT_LIB}_DEPEND_INCLUDE_DIRS}\")\n")
    endforeach()

endif()

if(ENABLE_EXPORT)
    # Stick build location in the central CMake user package registry, so that
    # it may be easily found.
    if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.7)
        export(PACKAGE ${PROJECTNAME})
    endif()
endif()

# Save a version file in the project's binary directory
set(VERSION_FILE "${PROJECT_BINARY_DIR}/${PROJECTNAME}ConfigVersion.cmake")
configure_file( "cmake/ProjectConfigVersion.cmake.in" ${VERSION_FILE} @ONLY )

include(UninstallTarget)
configure_uninstall_target(${CMAKE_CURRENT_BINARY_DIR}/Uninstall.cmake)
