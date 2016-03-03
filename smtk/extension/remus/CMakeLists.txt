set(remusExtSource MeshOperator.cxx)
set(remusExtHeaders MeshOperator.h)
smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/MeshOperator.sbt" remusOperatorXML)

add_library(smtkRemusExt ${remusExtSource})
target_link_libraries(smtkRemusExt
  LINK_PRIVATE
    RemusClient
    smtkCore
    ${Boost_LIBRARIES}
)

target_include_directories(smtkRemusExt PRIVATE ${REMUS_INCLUDE_DIRS})
smtk_export_header(smtkRemusExt smtkRemusExtExports.h)
smtk_public_headers(${remusExtHeaders})
smtk_install_library(smtkRemusExt)

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_library(smtkRemusExtPy MODULE smtkRemusExtPy.cxx)
  target_include_directories(smtkRemusExtPy PRIVATE ${PYTHON_INCLUDE_DIRS})
  target_link_libraries(smtkRemusExtPy
    smtkRemusExt
    ${PYTHON_LIBRARIES}
  )
  set_target_properties(smtkRemusExtPy PROPERTIES PREFIX "")
endif()


add_subdirectory(plugin)

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
