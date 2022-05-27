set(sources
  Registrar.cxx
  Registrar.h
  smtkAssignColorsView.cxx
  smtkAssignColorsView.h
)

if (ParaView_VERSION VERSION_GREATER_EQUAL "5.10.0")
  list(APPEND sources
    smtkMeshInspectorView.cxx
    smtkMeshInspectorView.h
    smtkDataSetInfoInspectorView.h
    smtkDataSetInfoInspectorView.cxx
)
endif ()

set(ui_files
  smtkAssignColorsParameters.ui
)

# for Qt methods
set(CMAKE_AUTOMOC 1)

smtk_add_plugin(smtkPQOperationViewsPlugin
  REGISTRAR
    smtk::extension::paraview::operators::Registrar
  MANAGERS
    smtk::view::Manager
  PARAVIEW_PLUGIN_ARGS
    VERSION "1.0"
    REQUIRED_PLUGINS smtkPQComponentsPlugin
    UI_FILES ${ui_files}
    SOURCES ${sources}
)

set(widget_library)
if (ParaView_VERSION VERSION_GREATER_EQUAL "5.10.0")
  set(widget_library smtkPQWidgetsExt)
else ()
  target_compile_definitions(smtkPQOperationViewsPlugin
    PRIVATE PARAVIEW_VERSION_59)
endif ()

target_link_libraries(smtkPQOperationViewsPlugin
  PRIVATE
    ${widget_library}
    vtkSMTKOperationsExt
    smtkPQComponentsExt
    smtkQtExt
    ParaView::pqComponents
    VTK::jsoncpp
)

smtk_export_header(smtkPQOperationViewsPlugin Exports.h)
