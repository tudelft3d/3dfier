
project( 3dfier )

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/" ${CMAKE_MODULE_PATH})

add_definitions(-std=c++11)

cmake_minimum_required(VERSION 2.6.2)
if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" VERSION_GREATER 2.6)
  if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER 2.8.3)
    cmake_policy(VERSION 2.8.4)
  else()
    cmake_policy(VERSION 2.6)
  endif()
endif()

set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true )

set(CMAKE_CXX_FLAGS "-O3")
 
if ( COMMAND cmake_policy )
  cmake_policy( SET CMP0003 NEW )  
endif()

# Boost
find_package( Boost REQUIRED locale chrono system)
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

# LIBLAS
find_package(libLAS REQUIRED)
find_package (LASzip QUIET)

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

include_directories( ${CGAL_INCLUDE_DIR} ${CGAL_3RD_PARTY_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${LIBLAS_INCLUDE_DIR} ${LASZIP_INCLUDE_DIR} ${YAMLCPP_INCLUDE_DIR} ${GDAL_INCLUDE_DIR})
link_directories(${YamlCpp_LIBRARY_DIRS})

# Creating entries for target: 3dfier
add_executable( 3dfier main.cpp io.cpp Map3d.cpp TopoFeature.cpp Bridge.cpp Separation.cpp Building.cpp Road.cpp Terrain.cpp Forest.cpp Water.cpp geomtools.cpp)
target_link_libraries( 3dfier ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES} ${BOOST_LIBRARIES} ${GDAL_LIBRARY} ${LIBLAS_LIBRARY} ${LASZIP_LIBRARY} ${YAMLCPP_LIBRARY} ${Boost_LIBRARIES})

install(TARGETS 3dfier DESTINATION bin)
