
#Build the actual code
add_subdirectory(attribute)
smtk_source_group(attribute)

add_subdirectory(model)
smtk_source_group(model)

set(smtk_headers ${attributeHeaders} ${modelHeaders})
set(smtk_srcs ${attributeSrcs} ${modelSrcs})

add_library(SMTKCore ${smtk_srcs} ${smtk_headers})
smtk_export_header(SMTKCore SMTKCoreExports.h)

#install the library
smtk_install_library(SMTKCore)

#setup the exports for the library when used from a build tree
export(PACKAGE SMTKCore)
export(TARGETS SMTKCore FILE SMTKCore-exports.cmake)

#add the Qt GUI code
if(SMTK_BUILD_QT)

  add_subdirectory(Qt)

  #we should at some point roll this into smtk_source_group
  get_directory_property(QAttrLibSrcs DIRECTORY Qt DEFINITION QAttrLibSrcs)
  smtk_prepend_string("Qt" QAttrLibSrcs ${QAttrLibSrcs})
  get_directory_property(QAttrLibMocHeaders DIRECTORY Qt DEFINITION QAttrLibMocHeaders)
  smtk_prepend_string("Qt" QAttrLibMocHeaders ${QAttrLibMocHeaders})
  get_directory_property(QAttrLibUIs DIRECTORY Qt DEFINITION QAttrLibUIs)
  smtk_prepend_string("Qt" QAttrLibUIs ${QAttrLibUIs})
  qt4_wrap_cpp(QAttrLibMocSrcs ${QAttrLibMocHeaders})
  qt4_wrap_ui(UI_BUILT_SOURCES ${QAttrLibUIs})

  source_group("Qt_Source" FILES ${QAttrLibSrcs} ${QAttrLibUIs})
  source_group("Qt_Generated" FILES ${QAttrLibMocSrcs} ${UI_BUILT_SOURCES})
  source_group("Qt_Header" FILES ${QAttrLibMocHeaders})

  set(qtsmtk_headers ${QAttrLibMocHeaders})
  set(qtsmtk_srcs
   ${QAttrLibSrcs}
   ${QAttrLibMocSrcs}
   ${UI_BUILT_SOURCES}
   )

  add_library(QtSMTK ${qtsmtk_srcs} ${qtsmtk_headers})
  #we need to add the location of the moc files to the include dir for
  #qtsmtk
  set_property(TARGET QtSMTK
               APPEND PROPERTY INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR})

  #set QtSMTK to link to qtGUI and include dirs
  qt4_use_modules(QtSMTK LINK_PUBLIC Gui)

  #set QtSMTK to publicly link to SMTKCore
  target_link_libraries(QtSMTK LINK_PUBLIC SMTKCore)
  smtk_export_header(QtSMTK QtSMTKExports.h)

  #install the library
  smtk_install_library(QtSMTK)

  #setup the exports for the library when used from a build tree
  export(PACKAGE QtSMTK)
  export(TARGETS QtSMTK SMTKCore FILE QtSMTK-exports.cmake)

endif()

#setup the exports for the library when used from an installed location
install(EXPORT SMTK-targets DESTINATION lib)
