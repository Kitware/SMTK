set(classes
  Registrar
)

vtk_module_add_module(smtkPVJobExt
  CLASSES ${classes}
  HEADERS_SUBDIR "smtk/extension/paraview/job"
)

vtk_module_link(smtkPVJobExt
  PUBLIC
    VTK::CommonDataModel
    smtkCore
    smtkQtExt
    ParaView::qttesting
    ParaView::pqComponents
)
