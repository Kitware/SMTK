find_package(VXL REQUIRED)
include(${VXL_CMAKE_DIR}/UseVXL.cmake)

set(srcs
  vtkTerrainExtractionFilter.cxx
  )

set(headers
  vtkTerrainExtractionFilter.h
)

set(${vtk-module}_NO_HeaderTest 1)
set(VTK_INSTALL_NO_HEADERS 1)
vtk_module_library(${vtk-module} ${srcs})

target_include_directories(${vtk-module}
  PRIVATE
    $<BUILD_INTERFACE:${SMTK_SOURCE_DIR}>
    $<BUILD_INTERFACE:${SMTK_BINARY_DIR}>
    $<INSTALL_INTERFACE:include/smtk/${SMTK_VERSION}>

    ${VXL_RPL_INCLUDE_DIR}
    ${VXL_VCL_INCLUDE_DIR}
    ${VXL_CORE_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(${vtk-module}
  LINK_PUBLIC
    vtkSMTKReaderExt # vtkLIDARReader
    vtkSMTKFilterExt
    smtkIOVTK # vtkLIDARWriter
  LINK_PRIVATE
    rtvl
    vnl
    rgtl
  )


# ... and make header compilation tests link properly:
smtk_install_library(${vtk-module})

# ... and install the export header
#has to be after the vtk_module_library
smtk_export_header(${vtk-module} Exports.h)

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  vtk_add_cs_wrapping(${vtk-module})
  smtk_install_library(${vtk-module}CS)
endif ()

smtk_public_headers(${headers})

if (SMTK_ENABLE_TESTING)
  # ... and make header compilation tests link properly:
  target_link_libraries(TestBuild_smtk_extension_vxl_vtk LINK_PRIVATE vtkCommonCore)
endif()
