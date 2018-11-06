
project( 3dfier )

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/" ${CMAKE_MODULE_PATH})

cmake_minimum_required(VERSION 2.6.2)
if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" VERSION_GREATER 2.6)
  if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER 2.8.3)
    cmake_policy(VERSION 2.8.4)
  else()
    cmake_policy(VERSION 2.6)
  endif()
endif()

set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true )

set(CMAKE_CXX_FLAGS "-O2")
 
if ( COMMAND cmake_policy )
  cmake_policy( SET CMP0003 NEW )  
endif()

# Boost
find_package( Boost REQUIRED locale chrono system filesystem program_options)
if ( NOT Boost_FOUND )
  message(SEND_ERROR "3dfier requires the Boost library")
  return()  
endif()
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

# GDAL
find_package( GDAL )
if ( NOT GDAL_FOUND )
  message(SEND_ERROR "3dfier requires the GDAL library")
endif()

# LASlib
find_package(LASlib REQUIRED)

# YamlCpp
find_package(YamlCpp REQUIRED)

# CGAL
find_package( CGAL QUIET COMPONENTS  )
if ( NOT CGAL_FOUND )
  message(SEND_ERROR "3dfier requires the CGAL library")
  return()  
endif()

# include helper file
include( ${CGAL_USE_FILE} )

include_directories( ${CMAKE_SOURCE_DIR}/thirdparty )

include_directories( ${CGAL_INCLUDE_DIR} ${CGAL_3RD_PARTY_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${LASLIB_INCLUDE_DIR} ${YAMLCPP_INCLUDE_DIR} ${GDAL_INCLUDE_DIR})
link_directories(${YamlCpp_LIBRARY_DIRS})

# build ptinpoly as static lib
add_library(ptinpoly STATIC thirdparty/ptinpoly.c)
set_target_properties(
  ptinpoly
  PROPERTIES C_STANDARD 11
)

# Creating entries for target: 3dfier
FILE(GLOB SRC_FILES src/*.cpp)
add_executable(3dfier ${SRC_FILES})
set_target_properties(
  3dfier
  PROPERTIES CXX_STANDARD 11
)

target_link_libraries( 3dfier ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES} ${GDAL_LIBRARY} ${LASLIB_LIBRARY} ${YAMLCPP_LIBRARY} Boost::program_options Boost::filesystem Boost::locale Boost::chrono ptinpoly)

install(TARGETS 3dfier DESTINATION bin)
