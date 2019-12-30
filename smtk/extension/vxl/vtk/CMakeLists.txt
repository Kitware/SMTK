find_package(VXL REQUIRED)
include(${VXL_CMAKE_DIR}/UseVXL.cmake)

set(classes
  vtkTerrainExtractionFilter)
set(private_headers
  rtvl_level_refine.h
  rtvl_level_refine.hxx)

vtk_module_add_module(vtkSMTKVXLExt
  CLASSES ${classes}
  PRIVATE_HEADERS ${private_headers}
  HEADERS_SUBDIR "smtk/extension/vxl/vtk")

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
