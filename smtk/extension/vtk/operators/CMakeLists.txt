
set(srcs
  vtkSMTKOperator.cxx
  BathymetryHelper.cxx
  BathymetryOperator.cxx
  ExportEdgesToVTK.cxx
  )

set(headers
  vtkSMTKOperator.h
  BathymetryHelper.h
  BathymetryOperator.h
  ExportEdgesToVTK.h
)

smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/BathymetryOperator.sbt" smtkVTKOperatorXML)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/ExportEdgesToVTK.sbt" smtkVTKOperatorXML)

set_source_files_properties(
  BathymetryHelper.cxx
  BathymetryOperator.cxx
  ExportEdgesToVTK.cxx
  PROPERTIES
    WRAP_EXCLUDE 1)

set(${vtk-module}_NO_HeaderTest 1)
set(VTK_INSTALL_NO_HEADERS 1)
vtk_module_library(${vtk-module} ${srcs})
target_link_libraries(${vtk-module}
      LINK_PUBLIC smtkCore vtksys)

target_include_directories(${vtk-module}
  PUBLIC
    $<BUILD_INTERFACE:${SMTK_SOURCE_DIR}>
    $<BUILD_INTERFACE:${SMTK_BINARY_DIR}>
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

add_subdirectory(plugin)
smtk_public_headers(${headers})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
# Potentially wrap in Python using VTK's wrapping methods, not shiboken.
# This makes the filters available for use with other VTK-wrapped filters.
#if (SMTK_ENABLE_PYTHON_WRAPPING AND VTK_WRAP_PYTHON)
# vtk_smtk_wrap_module_python3(${vtk-module} "${headers}")
#endif()
