set(classes
  Registrar
  smtkAssignColorsView
)
set(sources)

set(ui_files
  smtkAssignColorsParameters.ui
)

if (ParaView_VERSION VERSION_GREATER_EQUAL "5.10.0")
  list(APPEND classes
    smtkMeshInspectorView
    smtkCoordinateTransformView
    smtkDataSetInfoInspectorView
    smtkEditPropertiesView
  )
  list(APPEND ui_files
    smtkEditPropertiesView.ui
  )
endif ()

# for Qt methods
set(CMAKE_AUTOMOC 1)
set(CMAKE_AUTOUIC 1)

set(widget_library)
if (ParaView_VERSION VERSION_GREATER_EQUAL "5.10.0")
  set(widget_library smtkPQWidgetsExt)
else ()
  target_compile_definitions(smtkPQOperationViewsExt
    PRIVATE PARAVIEW_VERSION_59)
endif ()

vtk_module_add_module(smtkPQOperationViewsExt
  CLASSES ${classes}
  HEADERS_SUBDIR "smtk/extension/paraview/operators"
  SOURCES ${ui_files} ${rc_files}
)

vtk_module_link(smtkPQOperationViewsExt
  PUBLIC
    smtkCore
    smtkQtExt
    ParaView::RemotingViews
  PRIVATE
    ${widget_library}
    vtkSMTKOperationsExt
    smtkPQComponentsExt
    smtkQtExt
    ParaView::pqComponents
    VTK::jsoncpp
    smtkPQComponentsExt
    vtkSMTKSourceExt
    vtkSMTKWidgetsExt
    ParaView::pqApplicationComponents
    ParaView::pqComponents
    ParaView::pqWidgets
    ParaView::VTKExtensionsExtraction
    ParaView::VTKExtensionsFiltersGeneral
    ParaView::VTKExtensionsFiltersRendering
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::CommonExecutionModel
    VTK::CommonMath
    VTK::CommonTransforms
    VTK::InteractionWidgets
    VTK::RenderingCore
    VTK::RenderingOpenGL2
)

vtk_module_definitions(smtkPQOperationViewsExt
  PRIVATE
    QT_NO_KEYWORDS
)
