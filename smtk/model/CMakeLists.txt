# set up sources to build
set(modelSrcs
  Model.cxx
  ModelDomainItem.cxx
  Item.cxx
  GroupItem.cxx
  )

set(modelHeaders
  Model.h
  ModelDomainItem.h
  Item.h
  GridInfo.h
  GroupItem.h
  )

#install the headers
smtk_public_headers(${modelHeaders})

