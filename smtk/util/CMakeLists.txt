project(SMTK_Util)

# set up sources to build
set(utilSrcs
  Logger.cxx
  )

set(utilHeaders
  Logger.h
  )

#install the headers
smtk_public_headers(${utilHeaders})

if(SMTK_ENABLE_TESTING)
  add_subdirectory(Testing)
endif(SMTK_ENABLE_TESTING)
