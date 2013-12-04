
#Build the actual code
add_subdirectory(attribute)
smtk_source_group(attribute)

add_subdirectory(model)
smtk_source_group(model)

add_subdirectory(util)
smtk_source_group(util)

add_subdirectory(view)
smtk_source_group(view)

set(smtk_headers ${attributeHeaders} ${modelHeaders} ${utilHeaders} ${viewHeaders})
set(smtk_srcs ${attributeSrcs} ${modelSrcs} ${utilSrcs} ${viewSrcs})

add_library(SMTKCore ${smtk_srcs})
target_link_libraries(SMTKCore LINK_PUBLIC cJSON)
smtk_export_header(SMTKCore SMTKCoreExports.h)
if (NOT BUILD_SHARED_LIBS AND SMTK_BUILD_PYTHON_WRAPPINGS)
  # Set position-independent flag when other project libs are shared.
  set_target_properties(SMTKCore PROPERTIES POSITION_INDEPENDENT_CODE True)
endif()

if (BUILD_SHARED_LIBS)
  set_target_properties(SMTKCore PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

#install the library and exports the library when used from a build tree
smtk_install_library(SMTKCore DEPENDS cJSON)

#add the Qt GUI code
if(SMTK_BUILD_QT)

  add_subdirectory(Qt)

  #we should at some point roll this into smtk_source_group
  get_directory_property(QAttrLibSrcs DIRECTORY Qt DEFINITION QAttrLibSrcs)
  smtk_prepend_string("Qt" QAttrLibSrcs ${QAttrLibSrcs})
  get_directory_property(QAttrLibHeaders DIRECTORY Qt DEFINITION QAttrLibHeaders)
  smtk_prepend_string("Qt" QAttrLibHeaders ${QAttrLibHeaders})
  get_directory_property(QAttrLibUIs DIRECTORY Qt DEFINITION QAttrLibUIs)
  smtk_prepend_string("Qt" QAttrLibUIs ${QAttrLibUIs})

  qt4_wrap_ui(UI_BUILT_SOURCES ${QAttrLibUIs})
  qt4_add_resources(RCS_RESOURCES Qt/qtEntityItemModelIcons.qrc)

  source_group("Qt_Source" FILES ${QAttrLibSrcs} ${QAttrLibUIs})
  source_group("Qt_Generated" FILES ${QAttrLibMocSrcs} ${UI_BUILT_SOURCES} ${RCS_RESOURCES})
  source_group("Qt_Header" FILES ${QAttrLibMocHeaders})

  set(qtsmtk_headers ${QAttrLibHeaders})
  set(qtsmtk_srcs
   ${QAttrLibSrcs}
   ${UI_BUILT_SOURCES}
   ${RCS_RESOURCES}
   )

  add_library(QtSMTK ${qtsmtk_srcs})

  #let cmake do what qt4_wrap_cpp used to do automatically
  set_target_properties(QtSMTK PROPERTIES AUTOMOC TRUE
                        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")

  #we need to add the location of the moc files to the include dir for
  #qtsmtk
  set_property(TARGET QtSMTK
               APPEND PROPERTY INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR})

  #set QtSMTK to link to qtGUI and include dirs
  qt4_use_modules(QtSMTK LINK_PUBLIC Core Gui)

  #set QtSMTK to publicly link to SMTKCore
  target_link_libraries(QtSMTK LINK_PUBLIC SMTKCore)
  smtk_export_header(QtSMTK QtSMTKExports.h)

  #install the library and exports the library when used from a build tree
  smtk_install_library(QtSMTK DEPENDS SMTKCore)

  add_executable(qtViewTest Qt/qtViewTest.cxx)
  qt4_use_modules(qtViewTest LINK_PUBLIC Core Gui)
  target_link_libraries(qtViewTest LINK_PUBLIC  QtSMTK SMTKCore)

endif()

#add the VTK code
if(SMTK_BUILD_VTK)

  add_subdirectory(vtk)

  get_directory_property(VTKLibSrcs DIRECTORY vtk DEFINITION VTKLibSrcs)
  smtk_prepend_string("vtk" VTKLibSrcs ${VTKLibSrcs})
  get_directory_property(VTKLibHeaders DIRECTORY vtk DEFINITION VTKLibHeaders)
  smtk_prepend_string("vtk" VTKLibHeaders ${VTKLibHeaders})

  source_group("vtk_Source" FILES ${VTKLibSrcs} ${VTKLibUIs})
  source_group("vtk_Header" FILES ${VTKLibHeaders})

  set(vtksmtk_headers ${VTKLibHeaders})
  set(vtksmtk_srcs ${VTKLibSrcs})

  add_library(vtkSMTK ${vtksmtk_srcs})

  #set vtkSMTK to publicly link to SMTKCore and VTK
  target_link_libraries(vtkSMTK
    LINK_PUBLIC
      SMTKCore
    LINK_PRIVATE
      ${VTK_MODULES}
    )
  smtk_export_header(vtkSMTK vtkSMTKExports.h)

  # Add compile definitions to force linking to implementation modules
  set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})

  #install the library and exports the library when used from a build tree
  smtk_install_library(vtkSMTK DEPENDS SMTKCore)
endif()

install(FILES PublicPointerDefs.h DESTINATION include/smtk)

#setup the exports for the library when used from an installed location
install(EXPORT SMTK-targets DESTINATION lib)

#wrap everything
if(SMTK_BUILD_PYTHON_WRAPPINGS AND Shiboken_FOUND)
  sbk_wrap_library(SMTKCore
    GENERATOR_ARGS --avoid-protected-hack
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    LOCAL_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/attribute
      ${CMAKE_CURRENT_SOURCE_DIR}/model
      ${CMAKE_CURRENT_SOURCE_DIR}/util
      ${CMAKE_CURRENT_SOURCE_DIR}/view
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}
    TYPESYSTEM typesystem.xml
    HEADERS ${smtk_headers}
  )

  #get the location to copy the shiboken library so developers have an
  #easier time testing the python wrapping

  #first we have to resolve the path since shiboken library most likely
  #uses symlinks and we have to copy that version
  get_filename_component(shiboken_path "${SHIBOKEN_LIBRARY}" REALPATH)
  file(COPY "${shiboken_path}" DESTINATION "${SMTK_BINARY_DIR}")

  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/smtk.py"
                 "${SMTK_BINARY_DIR}/smtk.py" @ONLY)

  if(SMTK_ENABLE_TESTING)
    get_filename_component(SHIBOKEN_LIBRARY_PATH ${SHIBOKEN_LIBRARY} PATH)
    add_subdirectory(attribute/PythonTesting)
    add_subdirectory(model/PythonTesting)
    add_subdirectory(view/PythonTesting)
  endif(SMTK_ENABLE_TESTING)

  #todo we need to install this file in such a way that it can resolve
  #the location of shiboken, which is going to mean running fixup bundle
  #on the mac, and than fixing up the smtk.py package script

  get_filename_component(SHIBOKEN_LIBRARY_NAME ${SHIBOKEN_LIBRARY} NAME)
  install(CODE "set(LIBRARY_OUTPUT_PATH \"${CMAKE_INSTALL_PREFIX}/lib\")
                set(SHIBOKEN_LIBRARY \"${CMAKE_INSTALL_PREFIX}/lib/${SHIBOKEN_LIBRARY_NAME}\")
                configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/smtk.py ${CMAKE_INSTALL_PREFIX}/python/smtk.py )" )
endif()
