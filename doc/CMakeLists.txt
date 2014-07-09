####### SMTK Documentation

## Reference Documentation
#
# If we have doxygen, create reference API documentation for SMTK.
#
# This also generates documentation for third-party libraries
# cJSON and sparsehash which is referenced by SMTK's documentation.
#
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

## End-user Documentation
#
# If we have rst2html, create the user's guide for SMTK
# as an HTML document.

# Define a macro for processing reStructuredText files
# if docutils were found.
if (RST2HTML_EXECUTABLE)
  function(smtk_add_doc rstFile)
    set(options)
    set(oneValueArgs DESTINATION)
    set(multiValueArgs DEPENDS FIGURES)
    cmake_parse_arguments(rst "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
    set(figureList)
    foreach(FIG ${rst_FIGURES})
      add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FIG}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${FIG}"
        COMMAND ${CMAKE_COMMAND}
        ARGS
          -E copy_if_different
          "${CMAKE_CURRENT_SOURCE_DIR}/${FIG}"
          "${CMAKE_CURRENT_BINARY_DIR}/${FIG}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Copying figure ${FIG}"
      )
      list(APPEND figureList "${CMAKE_CURRENT_BINARY_DIR}/${FIG}")
    endforeach()
    add_custom_command(
      OUTPUT
        ${rstFile}.html
      DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/${rstFile}.rst
        ${SMTK_SOURCE_DIR}/doc/userguide.css
        ${rst_DEPENDS}
        ${figureList}
      COMMAND ${RST2HTML_EXECUTABLE}
      ARGS
        --stylesheet=${SMTK_SOURCE_DIR}/doc/userguide.css
        ${CMAKE_CURRENT_SOURCE_DIR}/${rstFile}.rst
        ${rstFile}.html
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating HTML for ${rstFile}"
    )
    add_custom_target(doc-${rstFile} ALL DEPENDS ${rstFile}.html)
    if (rst_DESTINATION)
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${rstFile}.html
        DESTINATION ${rst_DESTINATION}
        COMPONENT Development)
      install(
        FILES ${figureList}
        DESTINATION ${rst_DESTINATION}/figures
        COMPONENT Development)
    endif()
  endfunction()

  set(SMTK_USERGUIDE_DOCS
    userguide-model.rst
    userguide-attribute.rst
  )

  set(SMTK_USERGUIDE_FIGS
    figures/forwarding-bridge.svg
    figures/cursor-classes-with-inheritance.svg
  )

  # Add the top-level reStructuredText file.
  # All others are included by it.
  smtk_add_doc(userguide
    DEPENDS ${SMTK_USERGUIDE_DOCS}
    FIGURES ${SMTK_USERGUIDE_FIGS}
    DESTINATION share/doc/SMTK/userguide
  )
endif()

## Tutorial Documentation
#
add_subdirectory(tutorials)
