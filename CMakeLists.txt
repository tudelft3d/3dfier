cmake_minimum_required(VERSION 3.16)
project( 3dfier VERSION 1.5.0 )

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/" ${CMAKE_MODULE_PATH})

set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

if ( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE "Release" )
endif()

if ( COMMAND cmake_policy )
  cmake_policy( SET CMP0003 NEW )
  if ( POLICY CMP0167 )
    cmake_policy( SET CMP0167 NEW )
  endif()
endif()

# Boost
find_package( Boost REQUIRED COMPONENTS program_options filesystem locale chrono )

# GDAL
find_package( GDAL REQUIRED )
if ( TARGET GDAL::GDAL )
  set( GDAL_TARGET GDAL::GDAL )
else()
  set( GDAL_TARGET ${GDAL_LIBRARY} )
endif()

# PDAL
find_package( PDAL REQUIRED CONFIG )

# YamlCpp
find_package( yaml-cpp REQUIRED )
if ( TARGET yaml-cpp::yaml-cpp )
  set( YAML_CPP_TARGET yaml-cpp::yaml-cpp )
else()
  set( YAML_CPP_TARGET yaml-cpp )
endif()

# CGAL
find_package( CGAL REQUIRED COMPONENTS Core )

if(MSVC)
  add_definitions(-DNOMINMAX)
  add_definitions(/EHsc)
endif()

# Creating entries for target: 3dfier
FILE(GLOB SRC_FILES src/*.cpp)
add_executable(3dfier ${SRC_FILES})

target_include_directories( 3dfier PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty )

target_link_libraries( 3dfier
  CGAL::CGAL
  ${GDAL_TARGET}
  pdalcpp
  ${YAML_CPP_TARGET}
  Boost::program_options
  Boost::filesystem
  Boost::locale
  Boost::chrono
)

install(TARGETS 3dfier DESTINATION bin)
