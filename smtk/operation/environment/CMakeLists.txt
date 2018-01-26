set(environmentSrcs
  Environment.cxx
)

add_library(smtkOperationEnvironment ${environmentSrcs})
target_link_libraries(smtkOperationEnvironment PUBLIC smtkCore smtkEnvironment)
smtk_export_header(smtkOperationEnvironment Exports.h)
smtk_install_library(smtkOperationEnvironment)
