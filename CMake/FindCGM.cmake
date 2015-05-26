# FindCGM.cmake
#
# If you set the CGM_CFG CMake variable to point to a file named "cgm.make"
# (produced by CGM as part of any build or install), then the script will
# locate CGM assets for your package to use.
#
# This script defines the following CMake variables:
#   CGM_FOUND           defined when CGM is located, false otherwise
#   CGM_INCLUDE_DIRS    directories containing CGM headers
#   CGM_DEFINES         preprocessor definitions you should add to source files
#   CGM_LIBRARIES       paths to CGM library and its dependencies
#   CGM_HAVE_VERSION_H  true when the "cgm_version.h" header exists (v14.0 or later).
#
# Note that this script does not produce CGM_VERSION as that information
# is not available in the "cgm.make" configuration file that CGM creates.

find_file(CGM_CFG cgm.make DOC "Path to cgm.make configuration file")
if(CGM_CFG)
  set(CGM_FOUND 1)
  file(READ "${CGM_CFG}" CGM_CFG_DATA)
  ##
  ## Replace line continuations ('\' at EOL) so we don't have to parse them later
  ##
  string(REGEX REPLACE "\\\\\\\n" "" CGM_CFG_DATA "${CGM_CFG_DATA}")

  ##
  ## Find include directories
  ##
  string(REGEX MATCHALL "CGM_INT_INCLUDE =[^\\\n]*" _CGM_INCS "${CGM_CFG_DATA}")
  foreach(_CGM_INC ${_CGM_INCS})
    # Only use include directories specified by the *last*
    # occurrence of CGM_INT_INCLUDE in the config file:
    unset(CGM_INCLUDE_DIRS)

    string(REGEX REPLACE "-I" ";-I" _CGM_INC "${_CGM_INC}")
    foreach(_CGM_IDIR ${_CGM_INC})
      if ("${_CGM_IDIR}" MATCHES "^-I.*")
        string(REGEX REPLACE "-I" "" _CGM_IDIR "${_CGM_IDIR}")
        string(STRIP "${_CGM_IDIR}" _CGM_IDIR)
        list(APPEND CGM_INCLUDE_DIRS "${_CGM_IDIR}")
      endif()
    endforeach()
    # Alternately, one might:
    #list(APPEND CGM_INCLUDE_DIRS "${_CGM_INC}")
  endforeach()
  #message("CGM_INCLUDE_DIRS=\"${CGM_INCLUDE_DIRS}\"")

  ##
  ## Find preprocessor definitions
  ##
  string(REGEX MATCH "CGM_DEFINES =[^\\\n]*" CGM_DEFINES "${CGM_CFG_DATA}")
  string(REGEX REPLACE "CGM_DEFINES = ([^\\\n]*)" "\\1" CGM_DEFINES "${CGM_DEFINES}")

  ##
  ## Find CGM library directory(-ies)
  ##
  string(REGEX MATCHALL "CGM_INT_LDFLAGS =[^\\\n]*" _CGM_LDIRS "${CGM_CFG_DATA}")
  foreach(_CGM_LDIR ${_CGM_LDIRS})
    set(CGM_LIB_DIRS)
    string(REGEX REPLACE " -L" ";-L" _CGM_LDIR "${_CGM_LDIR}")
    string(REGEX REPLACE "CGM_INT_LDFLAGS = ([^\\\n]*)" "\\1" _CGM_LDIR "${_CGM_LDIR}")
    foreach(_CGM_LL ${_CGM_LDIR})
      if("${_CGM_LL}" MATCHES "^-L.*")
        string(REGEX REPLACE "-L" "" _CGM_LL "${_CGM_LL}")
        string(STRIP "${_CGM_LL}" _CGM_LL)
        list(APPEND CGM_LIB_DIRS "${_CGM_LL}")
      endif()
    endforeach()
  endforeach()

  ##
  ## Now add dependent library directories to CGM_LIB_DIRS
  ##
  string(REGEX MATCH "CGM_LDFLAGS =[^\\\n]*" _CGM_LDIR "${CGM_CFG_DATA}")
  string(REGEX REPLACE "CGM_LDFLAGS = ([^\\\n]*)" "\\1" _CGM_LDIR "${_CGM_LDIR}")
  string(REGEX REPLACE " -L" ";-L" _CGM_LDIR "${_CGM_LDIR}")
  set(_CGM_LDIRS)
  foreach(_CGM_LL ${_CGM_LDIR})
    if("${_CGM_LL}" MATCHES "^-L.*")
      string(REGEX REPLACE "-L" "" _CGM_LL "${_CGM_LL}")
      string(STRIP "${_CGM_LL}" _CGM_LL)
      list(APPEND _CGM_LDIRS "${_CGM_LL}")
    endif()
  endforeach()
  set(CGM_LIB_DIRS "${CGM_LIB_DIRS};${_CGM_LDIRS}")
  #message("${CGM_LIB_DIRS}")

  ##
  ## Find the CGM library and its dependencies
  ##
  string(REGEX MATCHALL "CGM_LIBS =[^\\\n]*" _CGM_LIBS "${CGM_CFG_DATA}")
  string(REGEX MATCHALL "-l[^ \t\n]+" _CGM_LIBS "${_CGM_LIBS}")
  foreach(_CGM_LIB ${_CGM_LIBS})
    string(REGEX REPLACE "-l" "" _CGM_LIB "${_CGM_LIB}")
    find_library(_CGM_LIB_LOC
      NAME "${_CGM_LIB}"
      # Cannot quote since it contains semicolons:
      PATHS ${CGM_LIB_DIRS}
      NO_DEFAULT_PATH
      NO_CMAKE_ENVIRONMENT_PATH
      NO_CMAKE_PATH
      NO_SYSTEM_ENVIRONMENT_PATH
      NO_CMAKE_SYSTEM_PATH
      )
    #message("Lib \"${_CGM_LIB}\" @ \"${_CGM_LIB_LOC}\" paths \"${CGM_LIB_DIRS}\"")
    if (_CGM_LIB_LOC)
      list(APPEND CGM_LIBRARIES "${_CGM_LIB_LOC}")
      unset(_CGM_LIB_LOC CACHE)
      unset(_CGM_LIB_LOC)
    else()
      message("Could not find ${_CGM_LIB} library (part of CGM)")
      unset(CGM_FOUND)
    endif()
  endforeach()
  #message("Libs ${CGM_LIBRARIES}")

  # Now detect the CGM version (or at least whether it is v14+).
  #
  # This is not as simple as it should be. CGM v14.0 and later
  # provide macros in cgm_version.h while earlier versions provide
  # "const int GeometryQueryTool::CGM_MAJOR_VERSION" which may
  # only be queried at run time, not compile time. We want to avoid
  # TRY_RUN since we may be cross-compiling.

  # Verify the file exists every time CMake is run, not just
  # the first time.
  if (CGM_VERSION_H AND NOT EXISTS "${CGM_VERSION_H}")
    unset(CGM_VERSION_H CACHE)
  endif()
  find_file(CGM_VERSION_H
    NAMES cgm_version.h
    PATHS ${CGM_INCLUDE_DIRS}
    DOC "Path to cgm_version.h if it exists"
    NO_DEFAULT_PATH
  )
  if (EXISTS "${CGM_VERSION_H}")
    set(CGM_HAVE_VERSION_H 1)
  else()
    unset(CGM_HAVE_VERSION_H)
  endif()
  mark_as_advanced(
    CGM_VERSION_H
    _CGM_LIB_LOC
  )

  ##
  ## Kill temporary variables
  ##
  unset(_CGM_INCS)
  unset(_CGM_INC)
  unset(_CGM_IDIR)
  unset(_CGM_LDIRS)
  unset(_CGM_LDIR)
  unset(_CGM_LL)
  unset(_CGM_LIBS)
  unset(_CGM_LIB)
  unset(_CGM_LIB_LOC)
else()
  unset(CGM_FOUND)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CGM
  REQUIRED_VARS CGM_INCLUDE_DIRS CGM_LIBRARIES
  VERSION_VAR CGM_VERSION
)
