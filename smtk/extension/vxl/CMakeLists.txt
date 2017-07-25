################################################################################
# Build vxl
################################################################################
if (PARAVIEW_BUILD_QT_GUI)
  add_subdirectory(operators)
  add_subdirectory(widgets)
endif()

# We need VXL for rgtl, vnl_algo, vnl, and vcl.
find_package(VXL REQUIRED)
if(NOT VXL_RPL_FOUND)
  message(FATAL_ERROR "vxl must be built with BUILD_RPL enabled")
endif(NOT VXL_RPL_FOUND)
if(NOT VXL_RPL_RGTL_FOUND)
  message(FATAL_ERROR "vxl must be built with BUILD_RPL_RGTL enabled")
endif(NOT VXL_RPL_RGTL_FOUND)
include(${VXL_CMAKE_DIR}/UseVXL.cmake)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${VXL_RPL_INCLUDE_DIR})
#include_directories(${VTKExtensions_SOURCE_DIR})

# Use the standard VXL include directories.
INCLUDE_DIRECTORIES(${VXL_VCL_INCLUDE_DIR} ${VXL_CORE_INCLUDE_DIR})

# Add link directories needed to use VXL.
LINK_DIRECTORIES(${VXL_LIBRARY_DIR})

# Build the Tensor Voting library.
add_subdirectory(rtvl)
