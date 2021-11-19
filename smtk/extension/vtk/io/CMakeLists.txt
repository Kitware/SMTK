set(srcs
  ImportAsVTKData.cxx
  RedirectOutput.cxx
  vtkLIDARPtsWriter.cxx # For terrain extraction
  mesh/ExportVTKData.cxx
  mesh/ImportVTKData.cxx
  mesh/MeshIOVTK.cxx
  mesh/PointCloudFromVTKFile.cxx
  mesh/StructuredGridFromVTKFile.cxx
  )

set(headers
  ImportAsVTKData.h
  RedirectOutput.h
  vtkLIDARPtsWriter.h # For terrain extraction
  mesh/ExportVTKData.h
  mesh/ImportVTKData.h
  mesh/MeshIOVTK.h
  mesh/PointCloudFromVTKFile.h
  mesh/StructuredGridFromVTKFile.h
  )

add_library(smtkIOVTK ${srcs})

if (SMTK_ENABLE_GDAL_SUPPORT AND TARGET VTK::IOGDAL)
  set(_smtk_extra_iovtk_libraries VTK::IOGDAL)
endif()

target_link_libraries(smtkIOVTK
  LINK_PUBLIC
    smtkCore
    vtkSMTKFilterExt
    VTK::CommonCore
    VTK::IOCore
  LINK_PRIVATE
    MOAB
    VTK::CommonDataModel
    VTK::CommonExecutionModel
    VTK::FiltersCore
    VTK::FiltersGeneral
    VTK::IOExodus
    VTK::IOGeometry
    VTK::IOImage
    VTK::IOLegacy
    VTK::IONetCDF
    VTK::IOPLY
    VTK::IOXML
    VTK::ImagingCore
    VTK::InteractionStyle
    VTK::RenderingCore
    VTK::RenderingFreeType
    VTK::RenderingOpenGL2
    VTK::vtksys
    ${_smtk_extra_iovtk_libraries}
)
vtk_module_autoinit(
  TARGETS smtkIOVTK
  MODULES
    vtkSMTKFilterExt
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::CommonExecutionModel
    VTK::FiltersCore
    VTK::FiltersGeneral
    VTK::IOCore
    VTK::IOExodus
    VTK::IOGeometry
    VTK::IOImage
    VTK::IOLegacy
    VTK::IONetCDF
    VTK::IOPLY
    VTK::IOXML
    VTK::ImagingCore
    VTK::InteractionStyle
    VTK::RenderingCore
    VTK::RenderingFreeType
    VTK::RenderingOpenGL2
    ${_smtk_extra_iovtk_libraries}
)

smtk_export_header(smtkIOVTK IOVTKExports.h)

target_include_directories(smtkIOVTK
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#install the headers
smtk_public_headers(smtkIOVTK ${headers})

# add to the VTK module export set
set(PROJECT_NAME_save "${PROJECT_NAME}")
set(PROJECT_NAME SMTKVTKModules)
smtk_install_library(smtkIOVTK)
set(PROJECT_NAME "${PROJECT_NAME_save}")

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
