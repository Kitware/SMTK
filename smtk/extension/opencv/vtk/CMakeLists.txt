set(srcs
  vtkSurfaceExtractorOperation.cxx
  )

set(headers
  vtkSurfaceExtractorOperation.h
)

set(${vtk-module}_NO_HeaderTest 1)
set(VTK_INSTALL_NO_HEADERS 1)
vtk_module_library(${vtk-module} ${srcs})
#Link explicitly to vtkSMTKOperationsExt, which can't be set in module.cmake
#because this is invoked by separate vtk_smtk_process_modules in polygon session
target_link_libraries(${vtk-module}
  LINK_PUBLIC
    smtkCore
    vtkSMTKOperationsExt
  LINK_PRIVATE
  )

target_include_directories(${vtk-module}
  PUBLIC
    $<BUILD_INTERFACE:${smtk_SOURCE_DIR}>
    $<BUILD_INTERFACE:${smtk_BINARY_DIR}>
    $<INSTALL_INTERFACE:include/smtk/${SMTK_VERSION}>)

# ... and make header compilation tests link properly:
smtk_install_library(${vtk-module})

# ... and install the export header
#has to be after the vtk_module_library
smtk_export_header(${vtk-module} Exports.h)

if (SMTK_ENABLE_PARAVIEW_SUPPORT)
  vtk_add_cs_wrapping(${vtk-module})
  smtk_install_library(${vtk-module}CS)
endif ()

smtk_public_headers(${vtk-module} ${headers})
