find_package(VXL REQUIRED)
include(${VXL_CMAKE_DIR}/UseVXL.cmake)

set(classes
  vtkTerrainExtractionFilter)
set(private_headers
  rtvl_level_refine.h
  rtvl_level_refine.hxx)

vtk_module_add_module(vtkSMTKVXLExt
  CLASSES ${classes}
  PRIVATE_HEADERS ${private_headers})

target_include_directories(vtkSMTKVXLExt
  PUBLIC
    ${VXL_RPL_INCLUDE_DIR}
    ${VXL_VCL_INCLUDE_DIR}
    ${VXL_CORE_INCLUDE_DIR}

  PRIVATE
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(vtkSMTKVXLExt
  PUBLIC
    smtkIOVTK # vtkLIDARWriter
    rtvl
    vnl
    rgtl
  )

# The list of public headers for a VTK module are stored as a "module property"
# (which is just a namespaced CMake TARGET property with some extra support for
# being different at install time). Ask for the headers and replace any
# in-source absolute paths with relative paths.
_vtk_module_get_module_property(vtkSMTKVXLExt
  PROPERTY headers
  VARIABLE vtk_headers)
string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" vtk_headers "${vtk_headers}")
smtk_public_headers(vtkSMTKVXLExt ${vtk_headers})
