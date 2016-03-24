#.rst:
# FindTriangle
# --------
#
# Find Triangle library
#
# Find the Triangle includes and library This module defines
#
# ::
#
#   TRIANGLE_INCLUDE_DIRS, where to find triangle.h.
#   TRIANGLE_LIBRARIES, libraries to link against to use triangle.
#   TRIANGLE_FOUND, If false, do not try to use TRIANGLE.
#
#
#
#
#
#=============================================================================

find_path(TRIANGLE_INCLUDE_DIR triangle.h
          DOC "The Triangle include directory")

set(TRIANGLE_NAMES ${TRIANGLE_NAMES} libtriangle triangle)
find_library(TRIANGLE_LIBRARY NAMES ${TRIANGLE_NAMES}
            DOC "The Triangle library")

# handle the QUIETLY and REQUIRED arguments and set TRIANGLE_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TRIANGLE
                                  REQUIRED_VARS TRIANGLE_LIBRARY
                                                TRIANGLE_INCLUDE_DIR
                                  VERSION_VAR TRIANGLE_VERSION_STRING)

if(TRIANGLE_FOUND)
  set( TRIANGLE_LIBRARIES ${TRIANGLE_LIBRARY} )
  set( TRIANGLE_INCLUDE_DIRS ${TRIANGLE_INCLUDE_DIR} )
endif()

mark_as_advanced(TRIANGLE_INCLUDE_DIR TRIANGLE_LIBRARY)