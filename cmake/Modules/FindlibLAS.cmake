###############################################################################
#
# CMake module to search for libLAS library
#
# On success, the macro sets the following variables:
# LIBLAS_FOUND       = if the library found
# LIBLAS_LIBRARIES   = full path to the library
# LIBLAS_INCLUDE_DIR = where to find the library headers also defined,
#                       but not for general use are
# LIBLAS_LIBRARY     = where to find the PROJ.4 library.
# LIBLAS_VERSION     = version of library which was found, e.g. "1.2.5"
#
# Copyright (c) 2009 Mateusz Loskot <mateusz@loskot.net>
#
# Module source: http://github.com/mloskot/workshop/tree/master/cmake/
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
###############################################################################
MESSAGE(STATUS "Searching for LibLAS ${LibLAS_FIND_VERSION}+ library")

IF(LIBLAS_INCLUDE_DIR)
  # Already in cache, be silent
  SET(LIBLAS_FIND_QUIETLY TRUE)
ENDIF()

IF(WIN32)
  IF(DEFINED ENV{LIBLAS_ROOT})
    SET(LIBLAS_ROOT_DIR $ENV{LIBLAS_ROOT})
    #MESSAGE(STATUS " FindLibLAS: trying LIBLAS using environment variable LIBLAS_ROOT=$ENV{LIBLAS_ROOT}")
  ELSE()
    SET(LIBLAS_ROOT_DIR c:/liblas)
    #MESSAGE(STATUS " FindLibLAS: trying LIBLAS using default location LIBLAS_ROOT=c:/liblas")
  ENDIF()
ENDIF()


FIND_PATH(LIBLAS_INCLUDE_DIR
  liblas.hpp
  PATH_PREFIXES liblas
  PATHS
  /usr/include
  /usr/local/include
  /tmp/lasjunk/include
  ${LIBLAS_ROOT_DIR}/include)

find_library(LIBLAS_LIBRARY
  NAMES liblas.dylib liblas
  PATHS
  /usr/lib
  /usr/local/lib
  /tmp/lasjunk/lib
  ${LIBLAS_ROOT_DIR}/lib
)





# Handle the QUIETLY and REQUIRED arguments and set LIBLAS_FOUND to TRUE
# if all listed variables are TRUE
#INCLUDE(FindPackageHandleStandardArgs)
#FIND_PACKAGE_HANDLE_STANDARD_ARGS(libLAS DEFAULT_MSG LIBLAS_LIBRARY LIBLAS_INCLUDE_DIR)