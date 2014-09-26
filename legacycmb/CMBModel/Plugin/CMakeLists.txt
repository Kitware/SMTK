include_directories(
  ${vtkDiscreteModel_SOURCE_DIR}
  ${vtkDiscreteModel_SOURCE_DIR}/Model
  )

set(Plugin_Src_Files
  vtkADHExporterOperatorClient.cxx
  vtkCMBImportBCFileOperatorClient.cxx
  vtkCMBModelBuilderClient.cxx
  vtkCMBModelBuilder2DClient.cxx
  vtkCMBMapToCMBModelClient.cxx
  vtkCMBModelReadOperatorClient.cxx
  vtkCMBModelWriterClient.cxx
  vtkCreateModelEdgesOperatorClient.cxx
  vtkEdgeSplitOperatorClient.cxx
  vtkMaterialOperatorClient.cxx
  vtkMergeOperatorClient.cxx
  vtkModelEntityGroupOperatorClient.cxx
  vtkModelEntityOperatorClient.cxx
  vtkSelectionSplitOperatorClient.cxx
  vtkSplitOperatorClient.cxx
  vtkCMBModelStateOperatorClient.cxx
  vtkCMBModelOmicronMeshInputWriterClient.cxx
  vtkModelEdgeOperatorClient.cxx
  vtkGeoTransformOperatorClient.cxx
  pqCMBModelPluginStarter.cxx
)

# We need this to call "vtkCmbDiscreteModelCS_Initialize(pm->GetInterpreter())"
# so that the vtkProcessModule's interpreter is exposed to these classes
# NOTE: This will not work in real cliet-server mode. According to Utkarsh,
# more magic needs to happen for client-server mode to work

#required can't use automoc here since plugin macros will attempt moccing too
qt4_wrap_cpp(MOC_SRCS pqCMBModelPluginStarter.h)
add_paraview_auto_start(IFACES IFACE_SRCS CLASS_NAME pqCMBModelPluginStarter
                        STARTUP onStartup
                        SHUTDOWN onShutdown)

add_paraview_plugin(CMBModel_Plugin "1.0"
  SERVER_MANAGER_XML smconfigConceptualModel.xml
  GUI_INTERFACES
    ${IFACES}
  SERVER_MANAGER_SOURCES
    vtkPVModelGeometryInformation.cxx
    vtkSIModelRepresentationProxy.cxx
    vtkPVCMBModelInformation.cxx
  GUI_SOURCES
    ${Plugin_Src_Files}
    ${IFACE_SRCS}
    ${MOC_SRCS}
)

target_link_libraries(CMBModel_Plugin
  LINK_PUBLIC
    vtkCmbDiscreteModel
    vtkDiscreteModel
  LINK_PRIVATE
    vtkCmbDiscreteModelCS
    vtkDiscreteModelCS
  )

target_include_directories(CMBModel_Plugin
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
  )

cmb_install_plugin(CMBModel_Plugin)
