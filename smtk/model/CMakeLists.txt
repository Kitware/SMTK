# set up sources to build
set(modelSrcs
  Arrangement.cxx
  BRepModel.cxx
  Entity.cxx
  GroupItem.cxx
  GridInfo.cxx
  GridInfo2D.cxx
  GridInfo3D.cxx
  Item.cxx
  Model.cxx
  Storage.cxx
  ModelDomainItem.cxx
  Tessellation.cxx
  )

if(NOT WIN32)
  set(modelSrcs ${modelSrcs} ExportJSON.cxx ImportJSON.cxx)
endif()

set(modelHeaders
  Arrangement.h
  BRepModel.h
  Entity.h
  Item.h
  GridInfo.h
  GridInfo2D.h
  GridInfo3D.h
  GroupItem.h
  Model.h
  Storage.h
  ModelDomainItem.h
  Tessellation.h
  )

if(NOT WIN32)
  set(modelHeaders ${modelHeaders} ExportJSON.h ImportJSON.h)
endif()

#install the headers
smtk_public_headers(${modelHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
