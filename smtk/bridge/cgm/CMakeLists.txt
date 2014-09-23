
set(CGMLibSrcs
  Bridge.cxx
  CAUUID.cxx
  Engines.cxx
  ExportSolid.cxx
  ImportSolid.cxx
  Operator.cxx
  ReadOperator.cxx
  TDUUID.cxx
)

set(CGMLibHeaders
  Bridge.h
  CAUUID.h
  Engines.h
  ExportSolid.h
  ImportSolid.h
  Operator.h
  ReadOperator.h
  TDUUID.h
)

add_definitions(${CGM_DEFINES})

add_library(cgmSMTK ${CGMLibSrcs})

#set cgmSMTK to publicly link to SMTKCore and CGM
target_link_libraries(cgmSMTK
  LINK_PUBLIC
    SMTKCore
  LINK_PRIVATE
    ${CGM_LIBRARIES}
  )
smtk_export_header(cgmSMTK cgmSMTKExports.h)

# Add compile definitions and include directories
set_property(TARGET cgmSMTK APPEND PROPERTY COMPILE_FLAGS ${CGM_DEFINES})
target_include_directories(cgmSMTK PUBLIC ${CGM_INCLUDE_DIRS})

# On Mac OS X, set the directory included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(cgmSMTK PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

#install the library and exports the library when used from a build tree
smtk_install_library(cgmSMTK DEPENDS SMTKCore)

if(SMTK_BUILD_PYTHON_WRAPPINGS AND Shiboken_FOUND)
  # Now handle wrapping dependent libraries
  set(smtk_cgm_headers ${cgmsmtk_headers})
  sbk_wrap_library(cgmSMTK
    GENERATOR_ARGS --avoid-protected-hack
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    LOCAL_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/attribute
      ${CMAKE_CURRENT_SOURCE_DIR}/cgm
      ${CMAKE_CURRENT_SOURCE_DIR}/model
      ${CMAKE_CURRENT_SOURCE_DIR}/util
      ${CMAKE_CURRENT_SOURCE_DIR}/view
      ${CMAKE_CURRENT_SOURCE_DIR}/vtk
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}
    TYPESYSTEM typesystem_cgm.xml
    HEADERS ${smtk_cgm_headers}
    DEPENDS SMTKCore
  )
  target_include_directories(cgmSMTKPython
    PRIVATE ${CGM_INCLUDE_DIRS})

    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/cgmsmtk.py"
      "${SMTK_BINARY_DIR}/cgmsmtk.py"
      @ONLY
    )

  #todo we need to install this file in such a way that it can resolve
  #the location of shiboken, which is going to mean running fixup bundle
  #on the mac, and than fixing up the cgmsmtk.py package script

  install(CODE "set(LIBRARY_OUTPUT_PATH \"${CMAKE_INSTALL_PREFIX}/lib\")
                configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/cgmsmtk.py ${CMAKE_INSTALL_PREFIX}/python/cgmsmtk.py )" )
endif()

# Operators which have XML descriptions in separate files
# need to have it encoded as a string in a header.
# We do not need the path to the generated header (appended
# to cgmOperatorXML) since the operators themselves include
# the header in their implementations.
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/ReadOperator.sbt" cgmOperatorXML)

# Install the headers
smtk_public_headers(${CGMLibHeaders})

# Utility programs using CGM:
add_executable(cgm-convert cgm-convert.cxx)
target_link_libraries(cgm-convert SMTKCore cgmSMTK ${CGM_LIBRARIES})
target_include_directories(cgm-convert PUBLIC ${CGM_INCLUDE_DIRS})

add_executable(facet-convert facet-convert.cxx)
target_link_libraries(facet-convert SMTKCore ${CGM_LIBRARIES})
target_include_directories(facet-convert PUBLIC ${CGM_INCLUDE_DIRS})

if (SMTK_BUILD_ParaView)
  include("${PARAVIEW_USE_FILE}")
  add_executable(vtk-to-cholla vtk-to-cholla.cxx)
  target_link_libraries(vtk-to-cholla
    SMTKCore
    vtkCommonDataModel
    vtkIOXML
    ${CGM_LIBRARIES}
  )
  target_include_directories(vtk-to-cholla PUBLIC ${CGM_INCLUDE_DIRS})
endif()

if (SMTK_ENABLE_TESTING)
  # ... and make header compilation tests link properly:
  target_link_libraries(TestBuild_smtk_bridge_cgm LINK_PRIVATE ${CGM_LIBRARIES})
  target_include_directories(TestBuild_smtk_bridge_cgm PUBLIC ${CGM_INCLUDE_DIRS})
  add_subdirectory(testing)
endif()
