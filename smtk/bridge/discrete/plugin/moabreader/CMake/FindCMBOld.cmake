# - Try to find CMB headers and libraries
#
# Usage of this module as follows:
#
#     find_package(CMB)
#
# Variables used by this module, they can change the default behavior and need
# to be set before calling find_package:
#
#  CMB_ROOT_DIR  Set this variable to the root installation of
#                 CMB if the module has problems finding
#                 the proper installation path.
#
#
# Search Variables defined by this module:
#
#  CMB_FOUND              System has CMB libs/headers
#  CMB_INCLUDE_DIRS       The location of CMB headers
#  CMB_LIBRARIES          The CMB library to link too
#

find_path(CMB_INCLUDE_DIR
  NAMES CMBTests.cmake
  HINTS
    ENV CMB_DIR
    ${CMB_ROOT_DIR}
  )

find_library(CMB_VTKEXTENSION_LIBRARY
  NAMES libCMBModel_Plugin
  HINTS
    ENV CMB_DIR
    ${CMB_ROOT_DIR}
)

find_library(CMB_MODEL_LIBRARY
  NAMES vtkCmbDiscreteModel
  HINTS
    ENV CMB_DIR
    ${CMB_ROOT_DIR}
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CMB DEFAULT_MSG
                                  CMB_VTKEXTENSION_LIBRARY
                                  CMB_MODEL_LIBRARY
                                  CMB_INCLUDE_DIR)

if(CMB_FOUND)
  set(CMB_INCLUDE_DIRS ${CMB_INCLUDE_DIR})
  set(CMB_LIBRARIES ${CMB_VTKEXTENSION_LIBRARY} ${CMB_MODEL_LIBRARY})
endif()

mark_as_advanced(CMB_INCLUDE_DIR CMB_VTKEXTENSION_LIBRARY CMB_MODEL_LIBRARY)

