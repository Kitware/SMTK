# - Try to find MOAB headers and libraries
#
# Usage of this module as follows:
#
#     find_package(MOAB)
#
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  MOAB_ROOT_DIR  Set this variable to the root installation of
#                            MOAB if the module has problems finding
#                            the proper installation path.
#
# Variables defined by this module:
#
#  MOAB_FOUND              System has MOAB libs/headers
#  MOAB_INCLUDE_DIRS       The location of MOAB headers
#
# Targets defined by this module:
#  MOAB
#

find_path(MOAB_ROOT_DIR
    NAMES include/MOAB/version.h
)

find_path(MOAB_INCLUDE_DIR
    NAMES moab/Core.hpp
    HINTS ${MOAB_ROOT_DIR}/include/
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(MOAB DEFAULT_MSG
    MOAB_INCLUDE_DIR)

if(MOAB_FOUND)
  #if not set deduce the ROOT_DIR from the INCLUDE_DIR
  if(NOT MOAB_ROOT_DIR)
    get_filename_component(MOAB_ROOT_DIR
                           "${MOAB_INCLUDE_DIR}"
                           DIRECTORY)
  endif()

  if(SMTK_USE_SYSTEM_MOAB)
    #if using system moab, determine if we have found a build or install
    #version of the library
    if(EXISTS ${MOAB_ROOT_DIR}/lib/MOABConfig.cmake)
      #found a build version of moab
      include(${MOAB_ROOT_DIR}/lib/MOABConfig.cmake)
      include(${MOAB_ROOT_DIR}/lib/MOABTargets.cmake)
    elseif(EXISTS ${MOAB_ROOT_DIR}/lib/cmake/MOAB/MOABConfig.cmake)
      #found an install version of moab
      include(${MOAB_ROOT_DIR}/lib/cmake/MOAB/MOABConfig.cmake)
      include(${MOAB_ROOT_DIR}/lib/cmake/MOAB/MOABTargets.cmake)
    endif()
  else()
      #Use the config files provided by the third-party version of moab
      include(MOABConfig.cmake)
      include(MOABTargets.cmake)
  endif()

  #Certain version of moab define a scope variable called BUILD_SHARED_LIBS
  #which hides the cache version of said variable. So we defend against
  #this by unsetting the variable
  unset(BUILD_SHARED_LIBS)


  set_target_properties(MOAB PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${MOAB_INCLUDE_DIR}"
      )
endif()

set(MOAB_INCLUDE_DIRS ${MOAB_INCLUDE_DIR})
mark_as_advanced(
    MOAB_ROOT_DIR
    MOAB_INCLUDE_DIR
    )
