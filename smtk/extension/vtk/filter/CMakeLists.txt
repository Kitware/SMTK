option(SMTK_ENABLE_OPENCV "Enable OpenCV" ON)

set( openCVFilterSource )
set( openCVFilterHeader )

if(SMTK_ENABLE_OPENCV)
  find_package(OpenCV REQUIRED)

  set( openCVFilterSource
        vtkGrabCutFilter.cxx
        vtkImageClassFilter.cxx
        vtkWatershedFilter.cxx )
  set( openCVFilterHeader
        vtkGrabCutFilter.h
        vtkImageClassFilter.h
        vtkOpenCVHelper.h
        vtkWatershedFilter.h )
endif()

set(srcs
   vtkCMBApplyBathymetryFilter.cxx
   vtkCleanPolylines.cxx
   vtkDEMToMesh.cxx
   vtkExtractLine.cxx
   vtkImageSpacingFlip.cxx
   vtkPolyDataStatsFilter.cxx
   ${openCVFilterSource}
  )

set(headers
   vtkCMBApplyBathymetryFilter.h
   vtkCleanPolylines.h
   vtkDEMToMesh.h
   vtkExtractLine.h
   vtkImageSpacingFlip.h
   vtkPolyDataStatsFilter.h
   ${openCVFilterHeader}
)

set(${vtk-module}_NO_HeaderTest 1)
set(VTK_INSTALL_NO_HEADERS 1)
vtk_module_library(${vtk-module} ${srcs})
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

if(SMTK_ENABLE_OPENCV)
  target_link_libraries(${vtk-module} LINK_PRIVATE vtkIOImage ${OpenCV_LIBS})
endif()
smtk_public_headers(${headers})

if (SMTK_ENABLE_TESTING)
  # ... and make header compilation tests link properly:
  target_link_libraries(TestBuild_smtk_extension_vtk_filter LINK_PRIVATE vtkCommonCore)
endif()
