####### SMTK Documentation

## Reference Documentation
#
# If we have doxygen, create reference API documentation for SMTK.
#
if(DOXYGEN_FOUND)
  file(MAKE_DIRECTORY "${smtk_BINARY_DIR}/doc/reference")
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/smtk.doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/smtk.doxyfile
    @ONLY
  )

  add_custom_command(
    OUTPUT ${smtk_BINARY_DIR}/doc/reference/smtk.tags
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/smtk.doxyfile
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/reference"
    DEPENDS
      "${CMAKE_CURRENT_BINARY_DIR}/smtk.doxyfile"
    COMMENT "Generating SMTK API documentation with Doxygen" VERBATIM
  )
  if (SMTK_BUILD_DOCUMENTATION STREQUAL "always")
    add_custom_target(doc ALL
      DEPENDS ${smtk_BINARY_DIR}/doc/reference/smtk.tags
    )
  else()
    add_custom_target(doc
      DEPENDS ${smtk_BINARY_DIR}/doc/reference/smtk.tags
    )
  endif()
endif()

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
        -Q
        -b html
        "${sphinx_SOURCE_DIR}"
        "${sphinx_BUILD_DIR}/html"
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating HTML for ${sphinxTargetName}"
    )
    if (SMTK_BUILD_DOCUMENTATION STREQUAL "always")
      add_custom_target(doc-${sphinxTargetName} ALL DEPENDS "${sphinx_HTML_TOP}")
    else() # must be "manual"
      add_custom_target(doc-${sphinxTargetName} DEPENDS "${sphinx_HTML_TOP}")
    endif()
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
    userguide/administration.rst
    userguide/attribute/attribute-builder.rst
    userguide/attribute/concepts.rst
    userguide/attribute/evaluators.rst
    userguide/attribute/file-syntax.rst
    userguide/attribute/filtering-and-searching.rst
    userguide/attribute/index.rst
    userguide/bindings/customizing-pybind11-bindings.rst
    userguide/bindings/generating-pybind11-bindings.rst
    userguide/bindings/index.rst
    userguide/bindings/python-overview.rst
    userguide/common/factory.rst
    userguide/common/generator.rst
    userguide/common/index.rst
    userguide/common/links.rst
    userguide/common/observers.rst
    userguide/common/threadpool.rst
    userguide/common/typecontainer.rst
    userguide/common/typemap.rst
    userguide/contributing/documentation.rst
    userguide/contributing/exposing.rst
    userguide/contributing/extending.rst
    userguide/contributing/index.rst
    userguide/contributing/organization.rst
    userguide/contributing/style.rst
    userguide/contributing/testing.rst
    userguide/contributing/todo.rst
    userguide/extension/index.rst
    userguide/extension/paraview/anatomy.rst
    userguide/extension/paraview/index.rst
    userguide/extension/paraview/lifecycle.rst
    userguide/extension/paraview/panels.rst
    userguide/extension/paraview/plugins.rst
    userguide/extension/paraview/representations.rst
    userguide/extension/paraview/selection.rst
    userguide/extension/paraview/widgets/design.rst
    userguide/extension/paraview/widgets/gallery.rst
    userguide/extension/paraview/widgets/index.rst
    userguide/extension/qt.rst
    userguide/extension/vtk.rst
    userguide/geometry/concepts.rst
    userguide/geometry/index.rst
    userguide/graph/concepts.rst
    userguide/graph/filtering-and-searching.rst
    userguide/graph/index.rst
    userguide/graph/nodes-and-arcs.rst
    userguide/mesh/concepts.rst
    userguide/mesh/index.rst
    userguide/mesh/IO.rst
    userguide/model/concepts.rst
    userguide/model/index.rst
    userguide/model/operators.rst
    userguide/model/ppg-file-format.rst
    userguide/model/property-names.rst
    userguide/model/session-discrete.rst
    userguide/model/session-exodus.rst
    userguide/model/session-polygon.rst
    userguide/model/sessions.rst
    userguide/model/user-interface.rst
    userguide/obtain-build-install.rst
    userguide/operation/index.rst
    userguide/operation/operators.rst
    userguide/overview.rst
    userguide/plugin/index.rst
    userguide/plugin/plugins.rst
    userguide/project/concepts.rst
    userguide/project/index.rst
    userguide/resource/concepts.rst
    userguide/resource/filtering-and-searching.rst
    userguide/resource/index.rst
    userguide/simulation/index.rst
    userguide/string/concepts.rst
    userguide/string/index.rst
    userguide/task/adaptors.rst
    userguide/task/classes.rst
    userguide/task/concepts.rst
    userguide/task/index.rst
    userguide/task/io.rst
    userguide/tips/index.rst
    userguide/tips/observers.rst
    userguide/view/concepts.rst
    userguide/view/index.rst
    userguide/view/phrases.rst
    userguide/view/views.rst

    tutorials/index.rst
    tutorials/add_a_session/index.rst
    tutorials/add_a_session/operators.rst
    tutorials/add_a_session/transcribing.rst
    tutorials/add_a_session/session_subclass.rst
    tutorials/add_a_session/entity_uuids.rst
    tutorials/python_first_steps/index.rst
    tutorials/cxx_first_steps/index.rst
    tutorials/implement_an_operator/index.rst
  )
  if (DOXYGEN_FOUND)
    # Need doxygen docs built if possible
    list(APPEND SMTK_USERGUIDE_DOCS
      ${smtk_BINARY_DIR}/doc/reference/smtk.tags)
  endif()

  set(SMTK_USERGUIDE_FIGS
    userguide/figures/forwarding-session.svg
    userguide/figures/entityref-classes-with-inheritance.svg
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
