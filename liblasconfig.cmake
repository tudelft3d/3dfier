# Configure libLAS package
#
# It defines the following variables
#  libLAS_FOUND = LIBLAS_FOUND - TRUE
#  libLAS_INCLUDE_DIRS - include directories for libLAS
#  libLAS_LIBRARY_DIRS - library directory
#  libLAS_LIBRARIES    - the libraries (as targets)
#  libLAS_BINARY_DIRS  - the directory for dll and utilites
#  libLAS_VERSION      - libLAS library version

message (STATUS "Reading ${CMAKE_CURRENT_LIST_FILE}")
# libLAS_VERSION is set by version file
message (STATUS "libLAS configuration, version " ${libLAS_VERSION})

# Tell the user project where to find our headers and libraries
get_filename_component (_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
get_filename_component (PROJECT_ROOT_DIR "${_DIR}/@PROJECT_ROOT_DIR@" ABSOLUTE)
set (libLAS_INCLUDE_DIRS "${PROJECT_ROOT_DIR}/include")
set (libLAS_LIBRARY_DIRS "${PROJECT_ROOT_DIR}/lib")
set (libLAS_BINARY_DIRS "${PROJECT_ROOT_DIR}/bin")

include ("${_DIR}/liblas-depends.cmake")
if(WIN32)
  set (libLAS_LIBRARIES liblas liblas_c)
else()
  set (libLAS_LIBRARIES las las_c)
endif()

# For backwards compatibility
set (LIBLAS_FOUND TRUE)