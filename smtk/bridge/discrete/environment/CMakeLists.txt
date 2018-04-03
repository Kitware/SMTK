set(environmentSrcs
  Environment.cxx
)

add_library(smtkDiscreteSessionEnvironment ${environmentSrcs})
target_link_libraries(smtkDiscreteSessionEnvironment
  PUBLIC
    smtkEnvironment
    smtkModelEnvironment
    smtkDiscreteSession
)
smtk_export_header(smtkDiscreteSessionEnvironment Exports.h)
smtk_install_library(smtkDiscreteSessionEnvironment)
