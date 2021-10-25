set(classes
  Registrar
  VisibilityBadge
  pqSMTKAttributePanel
  pqSMTKBehavior
  pqSMTKOperationPanel
  pqSMTKPipelineSelectionBehavior
  pqSMTKRenderResourceBehavior
  pqSMTKResource
  pqSMTKResourceBrowser
  pqSMTKResourcePanel
  pqSMTKResourceRepresentation
  pqSMTKWrapper
)

set(CMAKE_AUTOMOC 1)
set(CMAKE_AUTOUIC 1)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/resources")

set(iconFiles
  plugin/resources/pqEyeball.svg
  plugin/resources/pqEyeballClosed.svg)

set(uiFiles
  resources/pqSMTKOperationPanel.ui
)

foreach (iconFile ${iconFiles})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/${iconFile}"
    TYPE "_svg"
    HEADER_OUTPUT iconSvgHeader)
  list(APPEND iconDependencies ${iconSvgHeader})
endforeach()
add_custom_target("pvAppIconHeaders" DEPENDS ${iconDependencies})

vtk_module_add_module(smtkPQComponentsExt
  CLASSES ${classes}
  PRIVATE_HEADERS ${uiFiles}
  HEADERS_SUBDIR "smtk/extension/paraview/appcomponents")
add_dependencies(smtkPQComponentsExt "pvAppIconHeaders")
vtk_module_link(smtkPQComponentsExt
  PUBLIC
    smtkCore
    smtkQtExt
    Qt5::Widgets
)
