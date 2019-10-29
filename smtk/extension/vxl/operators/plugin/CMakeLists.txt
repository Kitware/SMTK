# Generate the ParaView plugin code for each view class
# added to the smtkVXLOperationViewsExt library above:
set(viewInterfaces)
set(viewSources)
foreach(src IN ITEMS smtkTerrainExtractionView)
  smtk_plugin_add_ui_view(
    CLASS_NAME ${src}
    VIEW_NAME ${src}
    HEADER_PATH "smtk/extension/vxl/operators/"
    INTERFACES interfaces
    SOURCES sources)
  list(APPEND viewInterfaces ${interfaces})
  list(APPEND viewSources ${sources})
endforeach()

paraview_add_plugin(smtkVXLOperationViewsPlugin
  VERSION "1.0"
  UI_INTERFACES ${viewInterfaces}
  SOURCES ${viewSources}
)
target_link_libraries(smtkVXLOperationViewsPlugin
  LINK_PUBLIC
    smtkVXLOperationViewsExt
)
