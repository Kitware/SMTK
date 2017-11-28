set(environmentSrcs
  Environment.cxx
)

add_library(smtkMeshEnvironment ${environmentSrcs})
target_link_libraries(smtkMeshEnvironment PUBLIC smtkCore smtkEnvironment)
smtk_install_library(smtkMeshEnvironment)
