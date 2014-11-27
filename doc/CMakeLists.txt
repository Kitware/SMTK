####### SMTK Documentation

## Reference Documentation
#
# If we have doxygen, create reference API documentation for SMTK.
#
# This also generates documentation for third-party libraries
# cJSON and sparsehash which is referenced by SMTK's documentation.
#
if(DOXYGEN_FOUND)
  file(MAKE_DIRECTORY "${SMTK_BINARY_DIR}/doc/reference")
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
      ${SMTK_SOURCE_DIR}/thirdparty/cJSON/cJSON.h
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

  add_custom_command(
    OUTPUT ${SMTK_BINARY_DIR}/doc/reference/smtk.tags
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/smtk.doxyfile
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/reference"
    DEPENDS
      "${CMAKE_CURRENT_BINARY_DIR}/smtk.doxyfile"
      "${SMTK_BINARY_DIR}/doc/reference/cjson.tags"
      "${SMTK_BINARY_DIR}/doc/reference/sparsehash.tags"
    COMMENT "Generating SMTK API documentation with Doxygen" VERBATIM
  )
  add_custom_target(doc
    DEPENDS ${SMTK_BINARY_DIR}/doc/reference/smtk.tags
  )
endif(DOXYGEN_FOUND)

## End-user Documentation
#
# If we have rst2html, create the user's guide for SMTK
# as an HTML document.

# Define a macro for processing reStructuredText files
# if docutils were found.
if (SPHINX_FOUND)
  function(smtk_add_doc sphinxTargetName)
    set(options)
    set(oneValueArgs DESTINATION SOURCE_DIR BUILD_DIR)
    set(multiValueArgs DEPENDS FIGURES)
    cmake_parse_arguments(sphinx "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
    if (NOT sphinx_SOURCE_DIR)
      set(sphinx_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}") # Reasonable default
    endif()
    if (NOT sphinx_BUILD_DIR)
      set(sphinx_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}") # Reasonable default
    endif()
    # Generate HTML version of docs
    set(sphinx_HTML_TOP "${CMAKE_CURRENT_BINARY_DIR}/${sphinx_BUILD_DIR}/html/index.html")
    add_custom_command(
      OUTPUT "${sphinx_HTML_TOP}"
      DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/conf.py
        ${sphinx_DEPENDS}
        ${figureList}
        COMMAND ${SPHINX_EXECUTABLE}
      ARGS
        -b html
        "${sphinx_SOURCE_DIR}"
        "${sphinx_BUILD_DIR}/html"
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating HTML for ${sphinxTargetName}"
    )
    add_custom_target(doc-${sphinxTargetName} DEPENDS "${sphinx_HTML_TOP}")
    if (sphinx_DESTINATION)
      install(
        DIRECTORY "${sphinx_BUILD_DIR}/html/"
        DESTINATION "${sphinx_DESTINATION}"
        COMPONENT Development)
      install(
        FILES ${figureList}
        DESTINATION "${sphinx_DESTINATION}/figures"
        COMPONENT Development)
    endif()
  endfunction()

  set(SMTK_USERGUIDE_DOCS
    index.rst
    userguide/index.rst
    userguide/attribute/index.rst
    userguide/attribute/concepts.rst
    userguide/attribute/file-syntax.rst
    userguide/model/index.rst
    userguide/model/concepts.rst
    userguide/model/bridges.rst
    userguide/model/property-names.rst
    userguide/contributing.rst
  )
  if (DOXYGEN_FOUND)
    # Need doxygen docs built if possible
    list(APPEND SMTK_USERGUIDE_DOCS
      ${SMTK_BINARY_DIR}/doc/reference/smtk.tags)
  endif()

  set(SMTK_USERGUIDE_FIGS
    userguide/figures/forwarding-bridge.svg
    userguide/figures/cursor-classes-with-inheritance.svg
    userguide/figures/ExampleAttributePanel.png
  )

  # Add the top-level reStructuredText file.
  # All others are included by it.
  smtk_add_doc(userguide
    DEPENDS ${SMTK_USERGUIDE_DOCS}
    FIGURES ${SMTK_USERGUIDE_FIGS}
    BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/user
    DESTINATION share/doc/SMTK/userguide
  )
endif()

## Tutorial Documentation
#
add_subdirectory(tutorials)
