###############################################################################
#
# CMake module to search for LASlib library
#
# On success, the macro sets the following variables:
# LASLIB_FOUND       = if the library found
# LASLIB_LIBRARY   = full path to the library
# LASLIB_INCLUDE_DIR = where to find the library headers also defined,
#                       but not for general use are
# LASLIB_LIBRARY     = where to find the library.
# LASLIB_VERSION     = version of library which was found, e.g. "1.2.5"
#
###############################################################################
MESSAGE(STATUS "Searching for LASlib library")

IF(LASLIB_INCLUDE_DIR)
  # Already in cache, be silent
  SET(LASLIB_FIND_QUIETLY TRUE)
ENDIF()

# IF(APPLE)
#   SET(LASLIB_ROOT_DIR /usr/local/)
#   MESSAGE(STATUS "Trying LasZip using default location LASLIB_ROOT=c:/laszip")
# ENDIF()

FIND_PATH(LASLIB_INCLUDE_DIR laszip.hpp
  PATH_PREFIXES laszip
  PATHS
  /usr/include
  /usr/local/include
  /usr/local/include/LASlib
  # ${LASLIB_ROOT_DIR}/include
  NO_DEFAULT_PATH)

SET(LASLIB_NAMES ${LASLIB_LIBRARY} LASlib)

FIND_LIBRARY(LASLIB_LIBRARY
  NAMES ${LASLIB_NAMES}
  PATHS
  /usr/lib
  /usr/local/lib
  /usr/local/lib/LASlib
  ${LASLIB_ROOT_DIR}/lib)


