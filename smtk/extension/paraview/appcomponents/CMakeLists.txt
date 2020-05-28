set(classes
  Registrar
  pqSMTKAttributePanel
  pqSMTKBehavior
  pqSMTKRenderResourceBehavior
  pqSMTKResource
  pqSMTKWrapper
  pqSMTKResourceBrowser
  pqSMTKResourceRepresentation
  pqSMTKResourcePanel
  VisibilityBadge)

set(CMAKE_AUTOMOC 1)

set(iconFiles
  plugin/resources/pqEyeball.svg
  plugin/resources/pqEyeballClosed.svg)

foreach (iconFile ${iconFiles})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/${iconFile}"
    TYPE "_svg"
    HEADER_OUTPUT iconSvgHeader)
  list(APPEND iconDependencies ${iconSvgHeader})
endforeach()
add_custom_target("pvAppIconHeaders" DEPENDS ${iconDependencies})

vtk_module_add_module(smtkPQComponentsExt
  CLASSES ${classes}
  HEADERS_SUBDIR "smtk/extension/paraview/appcomponents")
add_dependencies(smtkPQComponentsExt "pvAppIconHeaders")
vtk_module_link(smtkPQComponentsExt
  PUBLIC
    smtkCore
    smtkQtExt)
