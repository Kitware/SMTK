# Define a macro for processing reStructuredText files
# if docutils were found.
if (RST2HTML_EXECUTABLE)
  function(smtk_add_doc rstFiles)
    foreach (DOC ${rstFiles})
      add_custom_command(
        OUTPUT
          ${DOC}.html
        DEPENDS
          ${CMAKE_CURRENT_SOURCE_DIR}/${DOC}.rst
          ${SMTK_SOURCE_DIR}/doc/userguide.css
        COMMAND ${RST2HTML_EXECUTABLE}
        ARGS
          --stylesheet=${SMTK_SOURCE_DIR}/doc/userguide.css
          ${CMAKE_CURRENT_SOURCE_DIR}/${DOC}.rst
          ${DOC}.html
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating HTML for ${DOC}"
      )
      add_custom_target(run ALL DEPENDS ${DOC}.html)
    endforeach()
  endfunction()
endif()

# Now add tutorial directories
add_subdirectory(implement_an_operator)
