# set up sources to build
set(modelSrcs
  Arrangement.cxx
  BRepModel.cxx
  CellEntity.cxx
  Cursor.cxx
  DescriptivePhraseIterator.cxx
  Entity.cxx
  ExportJSON.cxx
  GroupItem.cxx
  GridInfo.cxx
  GridInfo2D.cxx
  GridInfo3D.cxx
  ImportJSON.cxx
  Item.cxx
  Model.cxx
  ShellEntity.h
  Storage.cxx
  ModelDomainItem.cxx
  Tessellation.cxx
  UseEntity.cxx
  Vertex.cxx
  )

set(modelHeaders
  Arrangement.h
  BRepModel.h
  CellEntity.h
  Cursor.h
  DescriptivePhraseIterator.h
  Entity.h
  EntityTypeBits.h
  ExportJSON.h
  FloatData.h
  ImportJSON.h
  IntegerData.h
  Item.h
  GridInfo.h
  GridInfo2D.h
  GridInfo3D.h
  GroupItem.h
  Model.h
  Storage.h
  ModelDomainItem.h
  ShellEntity.h
  StringData.h
  Tessellation.h
  UseEntity.h
  Vertex.h
  )

#install the headers
smtk_public_headers(${modelHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
