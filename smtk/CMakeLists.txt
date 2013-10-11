
#Build the actual code
add_subdirectory(attribute)
smtk_source_group(attribute)

add_subdirectory(model)
smtk_source_group(model)

add_subdirectory(util)
smtk_source_group(util)

set(smtk_headers ${attributeHeaders} ${modelHeaders} ${utilHeaders})
set(smtk_srcs ${attributeSrcs} ${modelSrcs} ${utilSrcs})

add_library(SMTKCore ${smtk_srcs})
smtk_export_header(SMTKCore SMTKCoreExports.h)

#install the library and exports the library when used from a build tree
smtk_install_library(SMTKCore)

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

  source_group("Qt_Source" FILES ${QAttrLibSrcs} ${QAttrLibUIs})
  source_group("Qt_Generated" FILES ${QAttrLibMocSrcs} ${UI_BUILT_SOURCES})
  source_group("Qt_Header" FILES ${QAttrLibMocHeaders})

  set(qtsmtk_headers ${QAttrLibHeaders})
  set(qtsmtk_srcs
   ${QAttrLibSrcs}
   ${UI_BUILT_SOURCES}
   )

  add_library(QtSMTK ${qtsmtk_srcs})

  #let cmake do what qt4_wrap_cpp used to do automatically
  set_target_properties(QtSMTK PROPERTIES AUTOMOC TRUE)

  #we need to add the location of the moc files to the include dir for
  #qtsmtk
  set_property(TARGET QtSMTK
               APPEND PROPERTY INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR})

  #set QtSMTK to link to qtGUI and include dirs
  qt4_use_modules(QtSMTK LINK_PUBLIC Gui)

  #set QtSMTK to publicly link to SMTKCore
  target_link_libraries(QtSMTK LINK_PUBLIC SMTKCore)
  smtk_export_header(QtSMTK QtSMTKExports.h)

  #install the library and exports the library when used from a build tree
  smtk_install_library(QtSMTK DEPENDS SMTKCore)

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
  endif(SMTK_ENABLE_TESTING)

  #todo we need to install this file in such a way that it can resolve
  #the location of shiboken, which is going to mean running fixup bundle
  #on the mac, and than fixing up the smtk.py package script

  install(CODE "set(LIBRARY_OUTPUT_PATH \"${CMAKE_INSTALL_PREFIX}/lib\")
                configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/smtk.py ${CMAKE_INSTALL_PREFIX}/python )" )
endif()