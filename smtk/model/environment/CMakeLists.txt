set(environmentSrcs
  Environment.cxx
)

add_library(smtkModelEnvironment ${environmentSrcs})
target_link_libraries(smtkModelEnvironment PUBLIC smtkCore smtkEnvironment)
smtk_export_header(smtkModelEnvironment Exports.h)
smtk_install_library(smtkModelEnvironment)
