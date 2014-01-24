
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

#add the ParaView code
if(SMTK_BUILD_ParaView)
  # Don't add the VTK subdirectory as it uses the vtk_module_library()
  # macro and vtk/module.cmake file. The pv_process_modules() macro
  # finds all subdirectories with module.cmake files automatically.
  #add_subdirectory(vtk)

  # However, we do need to generate the export macro in this directory,
  # even if the library is not built here.

  smtk_export_header(vtkSMTK vtkSMTKExports.h)
  smtk_export_header(pvSMTK pvSMTKExports.h)

  add_subdirectory(paraview)
endif()

if (SMTK_BUILD_CGM)

  add_subdirectory(cgm)
  smtk_source_group(cgm)

  get_directory_property(CGMLibSrcs DIRECTORY cgm DEFINITION CGMLibSrcs)
  smtk_prepend_string("cgm" CGMLibSrcs ${CGMLibSrcs})
  get_directory_property(CGMLibHeaders DIRECTORY cgm DEFINITION CGMLibHeaders)
  smtk_prepend_string("cgm" CGMLibHeaders ${CGMLibHeaders})

  source_group("cgm_Source" FILES ${CGMLibSrcs} ${CGMLibUIs})
  source_group("cgm_Header" FILES ${CGMLibHeaders})

  set(cgmsmtk_headers ${CGMLibHeaders})
  set(cgmsmtk_srcs ${CGMLibSrcs})

  add_library(cgmSMTK ${cgmsmtk_srcs})

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
  set_property(TARGET cgmSMTK APPEND PROPERTY INCLUDE_DIRECTORIES ${CGM_INCLUDE_DIRS})

  #install the library and exports the library when used from a build tree
  smtk_install_library(cgmSMTK DEPENDS SMTKCore)
endif(SMTK_BUILD_CGM)

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

  # Now handle wrapping dependent libraries
  if (SMTK_BUILD_CGM)
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

    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/cgmsmtk.py"
      "${SMTK_BINARY_DIR}/cgmsmtk.py"
      @ONLY
    )

    #if(SMTK_ENABLE_TESTING)
    #  add_subdirectory(cgm/PythonTesting)
    #  add_subdirectory(vtk/PythonTesting)
    #endif(SMTK_ENABLE_TESTING)

    #todo we need to install this file in such a way that it can resolve
    #the location of shiboken, which is going to mean running fixup bundle
    #on the mac, and than fixing up the cgmsmtk.py package script

    install(CODE "set(LIBRARY_OUTPUT_PATH \"${CMAKE_INSTALL_PREFIX}/lib\")
                  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/cgmsmtk.py ${CMAKE_INSTALL_PREFIX}/python/cgmsmtk.py )" )
  endif()
endif()
