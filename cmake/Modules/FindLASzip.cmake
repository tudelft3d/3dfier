###############################################################################
#
# CMake module to search for LASzip library
#
# On success, the macro sets the following variables:
# LASZIP_FOUND       = if the library found
# LASZIP_LIBRARIES   = full path to the library
# LASZIP_INCLUDE_DIR = where to find the library headers also defined,
#                       but not for general use are
# LASZIP_LIBRARY     = where to find the PROJ.4 library.
# LASZIP_VERSION     = version of library which was found, e.g. "1.2.5"
#
# Copyright (c) 2009 Mateusz Loskot <mateusz@loskot.net>
#
# Module source: http://github.com/mloskot/workshop/tree/master/cmake/
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
###############################################################################
MESSAGE(STATUS "Searching for LASzip ${LASzip_FIND_VERSION}+ library")

IF(LASZIP_INCLUDE_DIR)
  # Already in cache, be silent
  SET(LASZIP_FIND_QUIETLY TRUE)
ENDIF()

IF(WIN32)
  IF(DEFINED ENV{LASZIP_ROOT})
    SET(LASZIP_ROOT_DIR $ENV{LASZIP_ROOT})
    MESSAGE(STATUS "Trying LasZip using environment variable LASZIP_ROOT=$ENV{LASZIP_ROOT}")
  ELSE()
    SET(LASZIP_ROOT_DIR c:/laszip)
    MESSAGE(STATUS "Trying LasZip using default location LASZIP_ROOT=c:/laszip")
  ENDIF()
ENDIF()


FIND_PATH(LASZIP_INCLUDE_DIR laszip.hpp
  PATH_PREFIXES laszip
  PATHS
  /usr/include
  /usr/local/include
  ${LASZIP_ROOT_DIR}/include
  NO_DEFAULT_PATH)

SET(LASZIP_NAMES ${LASZIP_LIBRARY} laszip)

FIND_LIBRARY(LASZIP_LIBRARY
  NAMES ${LASZIP_NAMES}
  PATHS
  /usr/lib
  /usr/local/lib
  ${LASZIP_ROOT_DIR}/lib)


