project(Slctk_Attribute)

set(headers
  Attribute.h
  AttributeRefItem.h
  AttributeRefItemDefinition.h
  AttributeSection.h
  AttributeReader.h
  AttributeWriter.h
  Item.h
  ItemDefinition.h
  Definition.h
  DirectoryItem.h
  DirectoryItemDefinition.h
  DoubleItem.h
  DoubleItemDefinition.h
  FileItem.h
  FileItemDefinition.h
  GroupItem.h
  GroupItemDefinition.h
  GroupSection.h
  InstancedSection.h
  IntItem.h
  IntItemDefinition.h
  Manager.h
  ModelEntitySection.h
  RootSection.h
  Section.h
  SimpleExpressionSection.h
  StringItem.h
  StringItemDefinition.h
  ValueItem.h
  ValueItemDefinition.h
  ValueItemDefinitionTemplate.h
  ValueItemTemplate.h
  VoidItem.h
  VoidItemDefinition.h
  XmlDocV1Parser.h
  XmlV1StringWriter.h
  )

set(attribute_srcs
  Attribute.cxx
  AttributeRefItem.cxx
  AttributeRefItemDefinition.cxx
  AttributeSection.cxx
  AttributeReader.cxx
  AttributeWriter.cxx
  Item.cxx
  ItemDefinition.cxx
  Definition.cxx
  DirectoryItem.cxx
  DirectoryItemDefinition.cxx
  DoubleItem.cxx
  DoubleItemDefinition.cxx
  FileItem.cxx
  FileItemDefinition.cxx
  GroupItem.cxx
  GroupItemDefinition.cxx
  GroupSection.cxx
  InstancedSection.cxx
  IntItem.cxx
  IntItemDefinition.cxx
  Manager.cxx
  ModelEntitySection.cxx
  RootSection.cxx
  Section.cxx
  SimpleExpressionSection.cxx
  StringItem.cxx
  StringItemDefinition.cxx
  ValueItem.cxx
  ValueItemDefinition.cxx
  VoidItem.cxx
  VoidItemDefinition.cxx
  XmlV1StringWriter.cxx
  XmlDocV1Parser.cxx
)

# add model related classes
add_subdirectory(model)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/model)
get_directory_property(ModelSrcs DIRECTORY model DEFINITION ModelSrcs)
get_directory_property(ModelHeaders DIRECTORY model DEFINITION ModelHeaders)
source_group("model_Source" FILES ${ModelSrcs})
source_group("model_Header" FILES ${ModelHeaders})
set(headers ${headers} ${ModelHeaders})
set(attribute_srcs ${attribute_srcs} ${ModelSrcs})

# Option to build Qt ui compoments for attributes
option(Slctk_BUILD_QT "Build Qt GUI" OFF)

#add the Qt GUI code
if(Slctk_BUILD_QT)
  find_package(Qt4 REQUIRED)
  if(NOT QT4_FOUND)
    message(FATAL_ERROR "QT4 libraries are missing")
  endif()
  INCLUDE(${QT_USE_FILE})

  add_subdirectory(Qt)

  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Qt)
  include_directories(${CMAKE_CURRENT_BINARY_DIR}/Qt)

  set (slctkQtDepends ${QT_LIBRARIES})
  get_directory_property(QAttrLibSrcs DIRECTORY Qt DEFINITION QAttrLibSrcs)
  get_directory_property(QAttrLibMocHeaders DIRECTORY Qt DEFINITION QAttrLibMocHeaders)
  get_directory_property(QAttrLibUIS DIRECTORY Qt DEFINITION QAttrLibUIS)
  qt4_wrap_cpp(QAttrLibMocSrcs ${QAttrLibMocHeaders})
  qt4_wrap_ui(UI_BUILT_SOURCES ${QAttrLibUIs})

  source_group("Qt_Source" FILES ${QAttrLibSrcs} ${QAttrLibUIs})
  source_group("Qt_Generated" FILES ${QAttrLibMocSrcs})
  source_group("Qt_Header" FILES ${QAttrLibMocHeaders})
  source_group("Qt_Generated" FILES ${UI_BUILT_SOURCES})
endif()

IF(BUILD_SHARED_LIBS)
  SET(LIBTYPE SHARED)
ELSE(BUILD_SHARED_LIBS)
  SET(LIBTYPE STATIC)
ENDIF()

add_library(SlctkAttribute ${LIBTYPE}
   ${attribute_srcs} ${headers}
   ${QAttrLibSrcs} ${QAttrLibMocSrcs} ${QAttrLibMocHeaders}
   ${UI_BUILT_SOURCES})
target_link_libraries(SlctkAttribute ${slctkQtDepends})

slctk_set_includes(SlctkAttribute ${Slctk_Attribute_INCLUDE_DIRS})

#create the export header symbol exporting
slctk_export_header(SlctkAttribute AttributeExports.h)

#install the library
slctk_install_library(SlctkAttribute)

#install the headers
slctk_public_headers(${headers})

#setup the exports for the library when used from a build tree
export(PACKAGE SlctkAttribute)
export(TARGETS SlctkAttribute FILE SlctkAttribute-exports.cmake)

if(Slctk_ENABLE_TESTING)
  add_subdirectory(Testing)
endif(Slctk_ENABLE_TESTING)

