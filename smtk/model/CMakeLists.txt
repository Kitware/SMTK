# set up sources to build
set(ModelSrcs
  Model.cxx
  ModelDomainItem.cxx
  Item.cxx
  )

set(ModelHeaders
  Model.h
  ModelDomainItem.h
  Item.h
  )

#install the headers
smtk_public_headers(${ModelHeaders})

