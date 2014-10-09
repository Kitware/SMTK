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
  if(EXISTS ${MOAB_ROOT_DIR}/lib/MOABConfig.cmake)
    include(${MOAB_ROOT_DIR}/lib/MOABConfig.cmake)
  else()
    #now we create fake targets to be used
    include(${MOAB_ROOT_DIR}/lib/MOABTargets.cmake)
  endif()
endif()

set(MOAB_INCLUDE_DIRS ${MOAB_INCLUDE_DIR})
mark_as_advanced(
    MOAB_ROOT_DIR
    MOAB_INCLUDE_DIR
    )
