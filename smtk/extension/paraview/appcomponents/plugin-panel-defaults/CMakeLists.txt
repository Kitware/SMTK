set(headers)
set(interfaces
  DefaultConfiguration
)
set(sources)

set(classes
  DefaultConfiguration
)

foreach(class ${classes})
  list(APPEND sources ${class}.cxx)
  list(APPEND headers ${class}.h)
endforeach()

paraview_add_plugin(
  smtkDefaultConfigurationPlugin
  VERSION "1.0"
  REQUIRED_ON_SERVER
  REQUIRED_ON_CLIENT
  UI_INTERFACES ${interfaces}
  SOURCES ${sources}
)
target_link_libraries(smtkDefaultConfigurationPlugin
  LINK_PUBLIC
    smtkPQComponentsExt
  LINK_PRIVATE
    ParaView::pqCore
    ParaView::pqApplicationComponents
)
target_compile_definitions(smtkDefaultConfigurationPlugin PRIVATE QT_NO_KEYWORDS)
