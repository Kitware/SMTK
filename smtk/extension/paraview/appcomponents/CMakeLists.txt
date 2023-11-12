set(classes
  ApplicationConfiguration
  Registrar
  VisibilityBadge
  pqSMTKAttributePanel
  pqSMTKBehavior
  pqSMTKDock
  pqSMTKOperationPanel
  pqSMTKOperationParameterPanel
  pqSMTKOperationToolboxPanel
  pqSMTKPipelineSelectionBehavior
  pqSMTKRenderResourceBehavior
  pqSMTKResource
  pqSMTKResourceBrowser
  pqSMTKResourcePanel
  pqSMTKResourceRepresentation
  pqSMTKWrapper
  pqSMTKAppComponentsAutoStart
  pqSMTKGuiAppComponentsAutoStart
  pqSMTKCallObserversOnMainThreadBehavior
  pqSMTKCloseResourceBehavior
  pqSMTKGroupComponentsBehavior
  pqSMTKGroupingAutoStart
  pqSMTKCloseWithActiveOperationBehavior
  pqSMTKColorByToolBar
  pqSMTKColorByWidget
  pqSMTKDisplayAttributeOnLoadBehavior
  pqSMTKImportIntoResourceBehavior
  pqSMTKNewResourceBehavior
  pqSMTKOperationHintsBehavior
  pqSMTKRegisterImportersBehavior
  pqSMTKSaveOnCloseResourceBehavior
  pqSMTKSaveResourceBehavior
  pqSMTKSelectionFilterBehavior
  pqSMTKSubtractUI
  pqSMTKTaskPanel
  pqSMTKWorkletToolboxPanel
  vtkSMTKEncodeSelection

  # For testing
  pqToolboxEventTranslator
  pqToolboxEventPlayer
)

set(headers
  pqQtKeywordWrapping.h
  pqSMTKAttributeDock.h
  pqSMTKOperationDock.h
  pqSMTKOperationParameterDock.h
  pqSMTKOperationToolboxDock.h
  pqSMTKResourceDock.h
  pqSMTKTaskDock.h
  pqSMTKWorkletDock.h
)

set(CMAKE_AUTOMOC 1)
set(CMAKE_AUTOUIC 1)
set(CMAKE_AUTORCC 1)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/resources")

set(iconFiles
  resources/pqEyeball.svg
  resources/pqEyeballClosed.svg)

set(uiFiles
  resources/pqSMTKOperationPanel.ui
  resources/pqSMTKColorByWidget.ui
  resources/pqSMTKSelectionFilterBehavior.ui
)

set(rc_files
  resources/pqSMTKCloseResourceBehavior.qrc
  resources/pqSMTKSaveResourceBehavior.qrc
  resources/pqSMTKSelectionFilterBehavior.qrc
)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  list(APPEND classes
    pqSMTKPythonTrace
    pqSMTKExportSimulationBehavior
    pqSMTKImportOperationBehavior
  )
  list(APPEND rc_files
    resources/pqSMTKImportOperationBehavior.qrc
  )
endif()

foreach (iconFile ${iconFiles})
  get_filename_component(genFileBase "${iconFile}" NAME_WE)
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/${iconFile}"
    TYPE "_cpp" NAME "${genFileBase}_svg"
    HEADER_OUTPUT iconSvgHeader
    TARGET_OUTPUT iconTarget
  )
  list(APPEND pvAppHeaders ${iconSvgHeader})
  list(APPEND pvAppDependencies ${iconTarget})
endforeach()
add_custom_target("pvAppIconHeaders" DEPENDS ${pvAppDependencies})
set(pvAppDependencies ${_viewDependencies} PARENT_SCOPE)

vtk_module_add_module(smtkPQComponentsExt
  CLASSES ${classes}
  HEADERS ${headers}
  PRIVATE_HEADERS ${uiFiles}
  HEADERS_SUBDIR "smtk/extension/paraview/appcomponents"
  SOURCES ${rc_files})
add_dependencies(smtkPQComponentsExt "pvAppIconHeaders")
vtk_module_link(smtkPQComponentsExt
  PUBLIC
    smtkCore
    smtkQtExt
    Qt5::Widgets
)
if (ParaView_VERSION VERSION_LESS "5.10.0")
  target_compile_definitions(smtkPQComponentsExt
    PRIVATE PARAVIEW_VERSION_59)
endif ()
vtk_module_definitions(smtkPQComponentsExt PRIVATE QT_NO_KEYWORDS)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()
