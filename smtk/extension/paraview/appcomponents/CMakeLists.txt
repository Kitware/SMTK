###
# This directory contains 2 targets:
# + smtkPQComponentsExt, containing non-Qt (unwrapped) classes and Qt classes
# + smtkPQComponentsPlugin, a plugin exposing the above with an autostart object
###

set(PLUGIN_AUTOSTART_IFACES)
set(PLUGIN_AUTOSTART_IFACE_SRCS)
set(PLUGIN_ACTION_IFACES)
set(PLUGIN_ACTION_IFACE_SRCS)
set(PLUGIN_DOCK_IFACES)
set(PLUGIN_DOCK_IFACES_SRCS)

set(PQComponentsUnwrappedSrcs
)

set(PQComponentsUnwrappedHdrs
)

find_package(Qt5 REQUIRED COMPONENTS Core)

# set up sources to build
set(PQComponentsSrcs
  pqPluginSMTKViewBehavior.cxx
  pqSMTKAttributePanel.cxx
  pqSMTKBehavior.cxx
  pqSMTKCloseResourceBehavior.cxx
  pqSMTKColorByToolBar.cxx
  pqSMTKColorByWidget.cxx
  pqSMTKRenderResourceBehavior.cxx
  pqSMTKExportSimulationBehavior.cxx
  pqSMTKImportIntoResourceBehavior.cxx
  pqSMTKImportOperationBehavior.cxx
  pqSMTKModelRepresentation.cxx
  pqSMTKNewResourceBehavior.cxx
  pqSMTKOperationPanel.cxx
  pqSMTKPipelineSelectionBehavior.cxx
  pqSMTKRegisterImportersBehavior.cxx
  pqSMTKResource.cxx
  pqSMTKResourceBrowser.cxx
  pqSMTKResourcePanel.cxx
  pqSMTKSaveResourceBehavior.cxx
  pqSMTKSelectionFilterBehavior.cxx
  pqSMTKWrapper.cxx
)

set(PQComponentsHeaders
  pqPluginSMTKViewBehavior.h
  pqSMTKAttributePanel.h
  pqSMTKBehavior.h
  pqSMTKColorByToolBar.h
  pqSMTKColorByWidget.h
  pqSMTKCloseResourceBehavior.h
  pqSMTKRenderResourceBehavior.h
  pqSMTKExportSimulationBehavior.h
  pqSMTKImportIntoResourceBehavior.h
  pqSMTKImportOperationBehavior.h
  pqSMTKModelRepresentation.h
  pqSMTKNewResourceBehavior.h
  pqSMTKOperationPanel.h
  pqSMTKPipelineSelectionBehavior.h
  pqSMTKRegisterImportersBehavior.h
  pqSMTKResource.h
  pqSMTKResourceBrowser.h
  pqSMTKResourcePanel.h
  pqSMTKSaveResourceBehavior.h
  pqSMTKSelectionFilterBehavior.h
  pqSMTKWrapper.h
)

set(PQComponentsUIs
  resources/pqSMTKColorByWidget.ui
  resources/pqSMTKOperationPanel.ui
  resources/pqSMTKSelectionFilterBehavior.ui
)

set(PQComponentsResources
  resources/pqSMTKCloseResourceBehavior.qrc
  resources/pqSMTKImportOperationBehavior.qrc
  resources/pqSMTKSaveResourceBehavior.qrc
  resources/pqSMTKSelectionFilterBehavior.qrc
)

#install the headers
smtk_public_headers(
  smtkPQComponentsExt
  ${PQComponentsHeaders}
  ${PQComponentsUnwrappedHdrs}
)

pv_qt_wrap_cpp(MOC_BUILT_SOURCES ${PQComponentsHeaders})
pv_qt_wrap_ui(UI_BUILT_SOURCES ${PQComponentsUIs})
pv_qt_add_resources(RCS_SOURCES ${PQComponentsResources})

source_group("Qt_Source" FILES ${PQComponentsSrcs} )
source_group("Qt_Generated" FILES ${MOC_BUILT_SOURCES} )
source_group("Qt_Header" FILES ${PQComponentsHeaders})
source_group("Qt_Resources" FILES ${PQComponentsUIs})

add_library(smtkPQComponentsExt
  ${PQComponentsSrcs}
  ${PQComponentsUnwrappedSrcs}
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  ${RCS_SOURCES}
)

# Need to add the location of the moc files to the include dirs:
target_include_directories(smtkPQComponentsExt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#set smtkPQComponentsExt to publicly link to smtkCore
target_link_libraries(smtkPQComponentsExt LINK_PUBLIC
  pqApplicationComponents
  pqComponents

  # Server-side stuff because some stuff uses GetClientSideObject()
  vtkPVVTKExtensionsDefault
  smtkPVServerExt

  smtkQtExt
  Qt5::Core
)

smtk_export_header(smtkPQComponentsExt Exports.h)

set(PQComponentsPluginHdrs
  pqSMTKAppComponentsAutoStart.h
)

set(PQComponentsPluginSrcs
  pqSMTKAppComponentsAutoStart.cxx
)

add_paraview_auto_start(PLUGIN_AUTOSTART_IFACES PLUGIN_AUTOSTART_IFACE_SRCS
  CLASS_NAME pqSMTKAppComponentsAutoStart
  STARTUP startup
  SHUTDOWN shutdown
)

add_paraview_action_group(selectionFilterIfaces selectionFilterIfaceSrcs
  CLASS_NAME pqSMTKSelectionFilterBehavior
  GROUP_NAME "ToolBar/SMTK Selection Filters"
)
list(APPEND PLUGIN_ACTION_IFACES "${selectionFilterIfaces}")
list(APPEND PLUGIN_ACTION_IFACE_SRCS "${selectionFilterIfaceSrcs}")

add_paraview_toolbar(colorByToolBarIfaces colorByToolBarIfaceSrcs
  CLASS_NAME pqSMTKColorByToolBar
)
list(APPEND PLUGIN_ACTION_IFACES "${colorByToolBarIfaces}")
list(APPEND PLUGIN_ACTION_IFACE_SRCS "${colorByToolBarIfaceSrcs}")

add_pqproxy(modelRepIfaces modelRepIfaceSrcs
  TYPE pqSMTKModelRepresentation
  XML_GROUP representations
  XML_NAME SMTKModelCompositeRepresentation
)
list(APPEND PLUGIN_PROXY_IFACES ${modelRepIfaces})
list(APPEND PLUGIN_PROXY_IFACE_SRCS ${modelRepIfaceSrcs})

# Once the ParaView in CMB's superbuild includes commit
# SHA:000b6db4985a73899feb973185d8f60a2ef85f60 (03/20/2018), this macro can be
# replaced by ParaView's "add_pqproxy" macro.
macro(add_pqproxy_custom OUTIFACES OUTSRCS)
  set (arg_types)
  set (_doing)
  set (_active_index)
  foreach (arg ${ARGN})
    if ((NOT _doing) AND ("${arg}" MATCHES "^(TYPE|XML_GROUP|XML_NAME|XML_NAME_REGEX)$"))
      set (_doing "${arg}")
    elseif (_doing STREQUAL "TYPE")
      list(APPEND arg_types "${arg}")
      list(LENGTH arg_types _active_index)
      math(EXPR _active_index "${_active_index}-1")
      set (_type_${_active_index}_xmlgroup)
      set (_type_${_active_index}_xmlname)
      set (_doing)
    elseif (_doing STREQUAL "XML_GROUP")
      set (_type_${_active_index}_xmlgroup "${arg}")
      set (_doing)
    elseif (_doing STREQUAL "XML_NAME")
      set (_type_${_active_index}_xmlname "${arg}")
      set (_doing)
    elseif (_doing STREQUAL "XML_NAME_REGEX")
      set (_type_${_active_index}_xmlname_regex "${arg}")
      set (_doing)
    else()
      set (_doing)
    endif()
  endforeach()

  list(LENGTH arg_types num_items)
  math(EXPR max_index "${num_items}-1")
  set (ARG_INCLUDES)
  set (ARG_BODY)
  foreach (index RANGE ${max_index})
    list(GET arg_types ${index} arg_type)
    set (arg_xml_group "${_type_${index}_xmlgroup}")
    set (arg_xml_name "${_type_${index}_xmlname}")
    set (arg_xml_name_regex "${_type_${index}_xmlname_regex}")
    set (ARG_INCLUDES "${ARG_INCLUDES}#include \"${arg_type}.h\"\n")
    if (arg_xml_name_regex)
    set (ARG_INCLUDES "${ARG_INCLUDES}#include <QRegularExpression>\n")
    set (ARG_BODY "${ARG_BODY}
    if (QString(\"${arg_xml_group}\") == proxy->GetXMLGroup() &&
        QString(proxy->GetXMLName()).contains(QRegularExpression(\"${arg_xml_name_regex}\")))
        {
        return new ${arg_type}(regGroup, regName, proxy, server, NULL);
        }")
    else ()
    set (ARG_BODY "${ARG_BODY}
    if (QString(\"${arg_xml_group}\") == proxy->GetXMLGroup() &&
        QString(\"${arg_xml_name}\") == proxy->GetXMLName())
        {
        return new ${arg_type}(regGroup, regName, proxy, server, NULL);
        }")
    endif()
  endforeach()

  if (ARG_INCLUDES AND ARG_BODY)
    list(GET arg_types 0 ARG_TYPE)
    set (IMP_CLASS "${ARG_TYPE}ServerManagerModelImplementation")
    configure_file(${ParaView_CMAKE_DIR}/pqServerManagerModelImplementation.h.in
      ${CMAKE_CURRENT_BINARY_DIR}/${IMP_CLASS}.h @ONLY)
    configure_file(${ParaView_CMAKE_DIR}/pqServerManagerModelImplementation.cxx.in
      ${CMAKE_CURRENT_BINARY_DIR}/${IMP_CLASS}.cxx @ONLY)

    set (_moc_srcs)
    QT5_WRAP_CPP(_moc_srcs ${CMAKE_CURRENT_BINARY_DIR}/${IMP_CLASS}.h)

    set(${OUTIFACES} ${${OUTIFACES}} ${ARG_TYPE}ServerManagerModel) # don't add
                                        # the extra "Implementation" here.
    set(${OUTSRCS}
      ${${OUTSRCS}}
      ${_moc_srcs}
      ${CMAKE_CURRENT_BINARY_DIR}/${IMP_CLASS}.h
      ${CMAKE_CURRENT_BINARY_DIR}/${IMP_CLASS}.cxx
      )
  endif()

  unset (ARG_TYPE)
  unset (ARG_INCLUDES)
  unset (ARG_BODY)
endmacro()

add_pqproxy_custom(proxyIfaces proxyIfaceSrcs
  TYPE pqSMTKResource
  XML_GROUP sources
  XML_NAME SMTKSource

  TYPE pqSMTKResource
  XML_GROUP sources
  XML_NAME SMTKModelReader

  TYPE pqSMTKResource
  XML_GROUP sources
  XML_NAME SMTKModelCreator

  TYPE pqSMTKResource
  XML_GROUP sources
  XML_NAME_REGEX SMTKModelImporter*

  TYPE pqSMTKResource
  XML_GROUP sources
  XML_NAME SMTKAttributeReader

  TYPE pqSMTKResource
  XML_GROUP sources
  XML_NAME SMTKResource

  TYPE pqSMTKWrapper
  XML_GROUP smtk
  XML_NAME SMTKWrapper
)

list(APPEND PLUGIN_PROXY_IFACES ${proxyIfaces})
list(APPEND PLUGIN_PROXY_IFACE_SRCS ${proxyIfaceSrcs})

add_paraview_dock_window(attrPanelIfaces attrPanelIfaceSrcs
  CLASS_NAME pqSMTKAttributePanel
  DOCK_AREA Left
)
list(APPEND PLUGIN_DOCK_IFACES ${attrPanelIfaces})
list(APPEND PLUGIN_DOCK_IFACES_SRCS ${attrPanelIfaceSrcs})

add_paraview_dock_window(rsrcPanelIfaces rsrcPanelIfaceSrcs
  CLASS_NAME pqSMTKResourcePanel
  DOCK_AREA Left
)
list(APPEND PLUGIN_DOCK_IFACES ${rsrcPanelIfaces})
list(APPEND PLUGIN_DOCK_IFACES_SRCS ${rsrcPanelIfaceSrcs})

add_paraview_dock_window(operPanelIfaces operPanelIfaceSrcs
  CLASS_NAME pqSMTKOperationPanel
  DOCK_AREA Left
)
list(APPEND PLUGIN_DOCK_IFACES ${operPanelIfaces})
list(APPEND PLUGIN_DOCK_IFACES_SRCS ${operPanelIfaceSrcs})

pv_qt_wrap_cpp(PLUGIN_MOC_SRCS ${PQComponentsPluginHdrs})
add_paraview_plugin(
  smtkPQComponentsPlugin "1.0"
  GUI_INTERFACES
    ${PLUGIN_ACTION_IFACES}
    ${PLUGIN_AUTOSTART_IFACES}
    ${PLUGIN_PROXY_IFACES}
    ${PLUGIN_DOCK_IFACES}
  GUI_SOURCES
    ${PQComponentsPluginHdrs}
    ${PQComponentsPluginSrcs}
    ${PLUGIN_MOC_SRCS}
    ${PLUGIN_ACTION_IFACE_SRCS}
    ${PLUGIN_AUTOSTART_IFACE_SRCS}
    ${PLUGIN_PROXY_IFACE_SRCS}
    ${PLUGIN_DOCK_IFACES_SRCS}
)
set(SMTK_PLUGINS "${SMTK_PLUGINS};smtkPQComponentsPlugin" CACHE INTERNAL "")

target_link_libraries(smtkPQComponentsPlugin
  LINK_PUBLIC
    smtkPQComponentsExt
)
smtk_export_header(smtkPQComponentsPlugin PluginExports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(smtkPQComponentsExt)
smtk_install_library(smtkPQComponentsPlugin)
