###############################################################################
#
# CMake module to search for LAStools LASlib library
#
# On success, the macro sets the following variables:
# LASLIB_INCLUDE_DIR = where to find the library header files
# LASLIB_LIBRARY     = where to find the library file.
#
###############################################################################
MESSAGE(STATUS "Searching for LASlib library")

IF(LASLIB_INCLUDE_DIR)
  # Already in cache, be silent
  SET(LASLIB_FIND_QUIETLY TRUE)
ENDIF()

IF(WIN32)
  IF(DEFINED ENV{LASLIB_ROOT})
    SET(LASLIB_ROOT_DIR $ENV{LASLIB_ROOT})
    MESSAGE(STATUS "Trying LASlib using environment variable LASLIB_ROOT=$ENV{LASLIB_ROOT}")
  ELSE()
    SET(LASLIB_ROOT_DIR c:/lastools)
    MESSAGE(STATUS "Trying LASlib using default location LASLIB_ROOT=c:/lastools")
  ENDIF()
ENDIF()


FIND_PATH(LASLIB_INCLUDE_DIR laszip.hpp
  PATHS
  /usr/include
  /usr/local/include
  ${LASLIB_ROOT_DIR}/include
  PATH_SUFFIXES LASlib
  NO_DEFAULT_PATH)

SET(LASLIB_NAMES ${LASLIB_LIBRARY} LASlib)

FIND_LIBRARY(LASLIB_LIBRARY
  NAMES ${LASLIB_NAMES}
  PATHS
  /usr/lib
  /usr/local/lib
  ${LASLIB_ROOT_DIR}/lib
  PATH_SUFFIXES LASlib)


