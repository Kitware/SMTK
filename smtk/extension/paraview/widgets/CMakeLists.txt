set(headers
  pqSMTKAttributeItemWidgetP.h)

set(classes
  Registrar
  pqSMTKAttributeItemWidget
  pqSMTKBoxItemWidget
  pqSMTKTransformWidget
  qtSimpleExpressionEvaluationView)

set(CMAKE_AUTOMOC 1)

vtk_module_add_module(smtkPQWidgetsExt
  CLASSES ${classes}
  HEADERS ${headers}
  HEADERS_SUBDIR "smtk/extension/paraview/widgets")
vtk_module_link(smtkPQWidgetsExt
  PUBLIC
    smtkCore
    smtkQtExt
  PRIVATE
    vtkSMTKWidgetsExt)

if (SMTK_ENABLE_TESTING)
add_subdirectory(testing)
endif()
