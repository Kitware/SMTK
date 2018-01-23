set(environmentSrcs
  Environment.cxx
)

add_library(smtkResourceEnvironment ${environmentSrcs})
target_link_libraries(smtkResourceEnvironment PUBLIC smtkCore smtkEnvironment)
smtk_install_library(smtkResourceEnvironment)
