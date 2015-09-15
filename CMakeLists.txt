
project( 3dfier )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# C++11
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

cmake_minimum_required(VERSION 2.6.2)
if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" VERSION_GREATER 2.6)
  if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER 2.8.3)
    cmake_policy(VERSION 2.8.4)
  else()
    cmake_policy(VERSION 2.6)
  endif()
endif()

set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true )
 
if ( COMMAND cmake_policy )
  cmake_policy( SET CMP0003 NEW )  
endif()

# Boost
find_package( Boost REQUIRED )
if ( NOT Boost_FOUND )
  message(SEND_ERROR "3dfier requires the Boost library")
  return()  
endif()

# GDAL
find_package( GDAL )
if ( NOT GDAL_FOUND )
  message(SEND_ERROR "3dfier requires the GDAL library")
endif()

# LIBLAS
find_package(libLAS 1.8 REQUIRED)
find_package(LASzip 2.0.1 QUIET NO_MODULE)

# yaml-cpp
find_package(PkgConfig)
pkg_check_modules(yaml-cpp REQUIRED yaml-cpp)

include_directories( ${GDAL_INCLUDE_DIR} ${libLAS_INCLUDE_DIRS} ${LASZIP_INCLUDE_DIR} ${yaml-cpp_INCLUDE_DIRS} , include )

# Creating entries for target: 3dfier

add_executable( 3dfier input.cpp main.cpp Polygon3d.cpp Map3d.cpp)
target_link_libraries(3dfier ${BOOST_LIBRARIES} ${GDAL_LIBRARY} ${libLAS_LIBRARIES} ${LASZIP_LIBRARY} ${yaml-cpp_LIBRARIES} )

