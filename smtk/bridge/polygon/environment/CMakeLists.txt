set(environmentSrcs
  Environment.cxx
)

add_library(smtkPolygonSessionEnvironment ${environmentSrcs})
target_link_libraries(smtkPolygonSessionEnvironment
  PUBLIC
    smtkEnvironment
    smtkModelEnvironment
    smtkPolygonSession
)
smtk_export_header(smtkPolygonSessionEnvironment Exports.h)
smtk_install_library(smtkPolygonSessionEnvironment)
