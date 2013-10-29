# set up sources to build
set(modelSrcs
  Arrangement.cxx
  BRepModel.cxx
  Link.cxx
  ExportJSON.cxx
  GroupItem.cxx
  ImportJSON.cxx
  Item.cxx
  Model.cxx
  ModelBody.cxx
  ModelDomainItem.cxx
  Tessellation.cxx
  )

set(modelHeaders
  Arrangement.h
  BRepModel.h
  Link.h
  ExportJSON.h
  ImportJSON.h
  Item.h
  GridInfo.h
  GridInfo2D.h
  GridInfo3D.h
  GroupItem.h
  Model.h
  ModelBody.h
  ModelDomainItem.h
  Tessellation.h
  )

#install the headers
smtk_public_headers(${modelHeaders})

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
