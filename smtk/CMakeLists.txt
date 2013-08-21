
#Build the actual code
add_subdirectory(attribute)
get_directory_property(AttributeSrcs DIRECTORY attribute DEFINITION AttributeSrcs)
ms_prepend_string("attribute" AttributeSrcs ${AttributeSrcs})
get_directory_property(AttributeHeaders DIRECTORY attribute DEFINITION AttributeHeaders)
ms_prepend_string("attribute" AttributeHeaders ${AttributeHeaders})
source_group("attribute_Source" FILES ${AttributeSrcs})
source_group("attribute_Header" FILES ${AttributeHeaders})

add_subdirectory(model)
get_directory_property(ModelSrcs DIRECTORY model DEFINITION ModelSrcs)
ms_prepend_string("model" ModelSrcs ${ModelSrcs})
get_directory_property(ModelHeaders DIRECTORY model DEFINITION ModelHeaders)
ms_prepend_string("model" ModelHeaders ${ModelHeaders})
source_group("model_Source" FILES ${ModelSrcs})
source_group("model_Header" FILES ${ModelHeaders})

set(smtk_headers ${AttributeHeaders} ${ModelHeaders})
set(smtk_srcs ${AttributeSrcs} ${ModelSrcs})


add_library(SMTKCore ${smtk_srcs} ${smtk_headers})

smtk_set_includes(SMTKCore ${SMTK_Attribute_INCLUDE_DIRS})

#create the export header symbol exporting
smtk_export_header(SMTKCore SMTKCoreExports.h)

#install the library
smtk_install_library(SMTKCore)

#setup the exports for the library when used from a build tree
export(PACKAGE SMTKCore)
export(TARGETS SMTKCore FILE SMTKCore-exports.cmake)

#add the examples code
if(SMTK_ENABLE_EXAMPLES)
  add_subdirectory(Examples)
endif()

# Option to build Qt ui compoments for attributes
option(SMTK_BUILD_QT "Build Qt GUI" OFF)

#add the Qt GUI code
if(SMTK_BUILD_QT)
  find_package(Qt4 REQUIRED)
  if(NOT QT4_FOUND)
    message(FATAL_ERROR "QT4 libraries are missing")
  endif()
  INCLUDE(${QT_USE_FILE})

  add_subdirectory(Qt)

  set (smtkQtDepends ${QT_LIBRARIES})
  get_directory_property(QAttrLibSrcs DIRECTORY Qt DEFINITION QAttrLibSrcs)
  ms_prepend_string("Qt" QAttrLibSrcs ${QAttrLibSrcs})
  get_directory_property(QAttrLibMocHeaders DIRECTORY Qt DEFINITION QAttrLibMocHeaders)
  ms_prepend_string("Qt" QAttrLibMocHeaders ${QAttrLibMocHeaders})
  get_directory_property(QAttrLibUIs DIRECTORY Qt DEFINITION QAttrLibUIs)
  ms_prepend_string("Qt" QAttrLibUIs ${QAttrLibUIs})
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
  smtk_set_includes(QtSMTK ${QtSMTK_INCLUDE_DIRS})

  target_link_libraries(QtSMTK ${smtkQtDepends} LINK_PUBLIC SMTKCore)
  smtk_export_header(QtSMTK QtSMTKExports.h)

  #install the library
  smtk_install_library(QtSMTK)

  #setup the exports for the library when used from a build tree
  export(PACKAGE QtSMTK)
  export(TARGETS QtSMTK SMTKCore FILE QtSMTK-exports.cmake)

endif()

#setup the exports for the library when used from an installed location
install(EXPORT SMTK-targets DESTINATION lib)
