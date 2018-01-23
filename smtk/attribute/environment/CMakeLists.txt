set(environmentSrcs
  Environment.cxx
)

add_library(smtkAttributeEnvironment ${environmentSrcs})
target_link_libraries(smtkAttributeEnvironment PUBLIC smtkCore smtkEnvironment)
smtk_install_library(smtkAttributeEnvironment)
