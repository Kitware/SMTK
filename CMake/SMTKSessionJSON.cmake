# Given a list of filenames (opSpecs) containing JSON descriptions of
# a session, configure C++ source that encodes the JSON as a string.
# The resulting files are placed in the current binary directory and
# appended to genFiles.
include(EncodeCStringFunctions)
function(smtk_session_json opSpecs genFiles)
  foreach (opSpec ${opSpecs})
    get_filename_component(genFileBase "${opSpec}" NAME_WE)
    set(genFile "${CMAKE_CURRENT_BINARY_DIR}/${genFileBase}_json.h")
    #message("Writing ${genFileBase}_json.cxx (${opSpec})")
    configureFileAsCVariable("${opSpec}" "${genFile}" "${genFileBase}_json")
    set(${genFiles} ${${genFiles}} "${genFile}" PARENT_SCOPE)
  endforeach()
endfunction()
