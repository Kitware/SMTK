set(headers
  pqPointPickingVisibilityHelper.h
  pqSlicePropertyWidgetBaseP.h
  pqSMTKAttributeItemWidgetP.h
)

set(classes
  Registrar
  pqConePropertyWidget
  pqCrinklePropertyWidget
  pqPointPropertyWidget
  pqSlicePropertyWidget
  pqSlicePropertyWidgetBase
  pqSMTKAttributeItemWidget
  pqSMTKBoxItemWidget
  pqSMTKCoordinateFrameItemWidget
  pqSMTKTransformWidget
  pqSMTKConeItemWidget
  pqSMTKInfiniteCylinderItemWidget
  pqSMTKLineItemWidget
  pqSMTKPlaneItemWidget
  pqSMTKPointItemWidget
  pqSMTKSliceItemWidget
  pqSMTKSphereItemWidget
  pqSMTKSplineItemWidget
  qtSimpleExpressionEvaluationView
  vtkMeshInspectorRepresentation
)

set(ui_files
  resources/pqConePropertyWidget.ui
  resources/pqPointPropertyWidget.ui
  resources/pqSlicePropertyWidgetBase.ui
)
set(CMAKE_AUTOUIC_SEARCH_PATHS
  "${CMAKE_CURRENT_SOURCE_DIR}/resources")

set(rc_files
  qtWidgetsIcons.qrc)

# for Qt methods
set(CMAKE_AUTOMOC 1)
set(CMAKE_AUTOUIC 1)

vtk_module_add_module(smtkPQWidgetsExt
  CLASSES ${classes}
  HEADERS ${headers}
  HEADERS_SUBDIR "smtk/extension/paraview/widgets"
  SOURCES ${ui_files} ${rc_files}
)
vtk_module_link(smtkPQWidgetsExt
  PUBLIC
    smtkCore
    smtkQtExt
  PRIVATE
    smtkPQComponentsExt
    vtkSMTKSourceExt
    vtkSMTKWidgetsExt
    ParaView::RemotingViews
    ParaView::pqApplicationComponents
    ParaView::pqComponents
    ParaView::pqWidgets
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::CommonExecutionModel
    VTK::CommonMath
    VTK::CommonTransforms
    VTK::InteractionWidgets
    VTK::RenderingCore
    VTK::RenderingOpenGL2
)

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
