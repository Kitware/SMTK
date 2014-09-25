project(SMTK_View)

set(viewHeaders
  Attribute.h
  Base.h
  Group.h
  Instanced.h
  ModelEntity.h
  Root.h
  SimpleExpression.h
  )

set(viewSrcs
  Attribute.cxx
  Base.cxx
  Group.cxx
  Instanced.cxx
  ModelEntity.cxx
  Root.cxx
  SimpleExpression.cxx
)

#install the headers
smtk_public_headers(${viewHeaders})


if(SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif(SMTK_ENABLE_TESTING)
