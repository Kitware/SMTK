###### SMTK Documentation
#
# If we have doxygen, create reference API documentation for SMTK.
#
# This also generates documentation for third-party libraries
# cJSON and sparsehash which is referenced by SMTK's documentation.
#
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cjson.doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/cjson.doxyfile
    @ONLY
  )
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/sparsehash.doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/sparsehash.doxyfile
    @ONLY
  )
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/smtk.doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/smtk.doxyfile
    @ONLY
  )

  add_custom_command(
    OUTPUT ${SMTK_BINARY_DIR}/doc/reference/cjson.tags
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/cjson.doxyfile
    DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/cjson.doxyfile
      ${SMTK_SOURCE_DIR}/ThirdParty/cJSON/cJSON.h
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/reference"
    COMMENT "Generating cJSON API documentation with Doxygen" VERBATIM
    )

  add_custom_command(
    OUTPUT ${SMTK_BINARY_DIR}/doc/reference/sparsehash.tags
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/sparsehash.doxyfile
    DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/sparsehash.doxyfile
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/reference"
    COMMENT "Generating sparsehash API documentation with Doxygen" VERBATIM
    )

  add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/smtk.doxyfile
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/reference"
    DEPENDS
      "${CMAKE_CURRENT_BINARY_DIR}/smtk.doxyfile"
      "${SMTK_BINARY_DIR}/doc/reference/cjson.tags"
      "${SMTK_BINARY_DIR}/doc/reference/sparsehash.tags"
    COMMENT "Generating SMTK API documentation with Doxygen" VERBATIM
  )
endif(DOXYGEN_FOUND)
