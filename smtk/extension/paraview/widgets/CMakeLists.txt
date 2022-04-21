set(headers
  pqPointPickingVisibilityHelper.h
  pqSlicePropertyWidgetBaseP.h
  pqSMTKAttributeItemWidgetP.h
  pqSMTKInteractivePropertyWidget.h
)

set(classes
  Registrar
  pqConePropertyWidget
  pqPointPropertyWidget
  pqSMTKAttributeItemWidget
  pqSMTKBoxItemWidget
  pqSMTKTransformWidget
  pqSMTKConeItemWidget
  pqSMTKInfiniteCylinderItemWidget
  pqSMTKLineItemWidget
  pqSMTKPointItemWidget
  pqSMTKSphereItemWidget
  pqSMTKSplineItemWidget
  qtSimpleExpressionEvaluationView
)

set(ui_files
  resources/pqConePropertyWidget.ui
  resources/pqPointPropertyWidget.ui
)

if (ParaView_VERSION VERSION_GREATER_EQUAL "5.10")
  list(APPEND classes
    pqCrinklePropertyWidget
    pqSMTKCoordinateFrameItemWidget
    pqSMTKPlaneItemWidget
    pqSMTKSliceItemWidget
    pqSlicePropertyWidget
    pqSlicePropertyWidgetBase
    vtkMeshInspectorRepresentation
  )

  list(APPEND ui_files
    resources/pqSlicePropertyWidgetBase.ui
  )
endif ()

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
vtk_module_definitions(smtkPQWidgetsExt PRIVATE QT_NO_KEYWORDS)


if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
