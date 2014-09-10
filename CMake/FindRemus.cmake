# - Try to find Remus headers and libraries
#
# Usage of this module as follows:
#
#     find_package(Remus)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  REMUS_ROOT_DIR  Set this variable to the root installation of
#                            Remus if the module has problems finding
#                            the proper installation path.
#
# Variables defined by this module:
#
#  REMUS_FOUND              System has Remus libs/headers
#  REMUS_INCLUDE_DIRS        The location of Remus headers

find_path(REMUS_ROOT_DIR
    NAMES include/remus/version.h
)

find_path(REMUS_INCLUDE_DIR
    NAMES remus/version.h
    HINTS ${REMUS_ROOT_DIR}/include/
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Remus DEFAULT_MSG
    REMUS_INCLUDE_DIR
)

if(REMUS_FOUND)
  #now we create the import targets to be used
  include(${REMUS_ROOT_DIR}/lib/Remus-targets.cmake)

  #find boost since it could be in a location that isn't part of our
  #include paths
  #cmb_find_boost()

  #next find ZeroMQ since it could be in a location that isn't part of our
  #include paths
  find_package(ZeroMQ REQUIRED)

  include_directories(${Boost_INCLUDE_DIRS}
                      ${ZeroMQ_INCLUDE_DIRS})
endif()

set(REMUS_INCLUDE_DIRS ${REMUS_INCLUDE_DIR})

mark_as_advanced(
    REMUS_ROOT_DIR
    REMUS_INCLUDE_DIR
)
