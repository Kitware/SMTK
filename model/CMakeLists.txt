project(Slctk_Model)

set(headers
  Model.h
  ModelDomainItem.h
  Item.h
  )

set(model_srcs
  Model.cxx
  ModelDomainItem.cxx
  Item.cxx
)

add_library(SlctkModel ${model_srcs} ${headers})
target_link_libraries(SlctkModel)

slctk_set_includes(SlctkModel ${Slctk_Model_INCLUDE_DIRS})

#create the export header symbol exporting
slctk_export_header(SlctkModel ModelExports.h)

#install the library
slctk_install_library(SlctkModel)

#install the headers
slctk_public_headers(${headers})

#setup the exports for the library when used from a build tree
export(PACKAGE SlctkModel)
export(TARGETS SlctkModel FILE SlctkModel-exports.cmake)

if(Slctk_ENABLE_TESTING)
  # add_subdirectory(Testing)
endif(Slctk_ENABLE_TESTING)

