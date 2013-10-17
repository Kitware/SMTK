# set up sources to build
set(modelSrcs
  Model.cxx
  ModelDomainItem.cxx
  Item.cxx
  GridInfo2D.cxx
  GridInfo3D.cxx
  GroupItem.cxx
  )

set(modelHeaders
  Model.h
  ModelDomainItem.h
  Item.h
  GridInfo.h
  GridInfo2D.h
  GridInfo3D.h
  GroupItem.h
  )

#install the headers
smtk_public_headers(${modelHeaders})

