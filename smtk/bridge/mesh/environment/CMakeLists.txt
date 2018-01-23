set(environmentSrcs
  Environment.cxx
)

add_library(smtkMeshSessionEnvironment ${environmentSrcs})
target_link_libraries(smtkMeshSessionEnvironment PUBLIC smtkEnvironment smtkMeshSession)
smtk_export_header(smtkMeshSessionEnvironment Exports.h)
smtk_install_library(smtkMeshSessionEnvironment)
