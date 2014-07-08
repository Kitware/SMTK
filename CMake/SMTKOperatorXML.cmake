# Given a list of filenames (opSpecs) containing XML descriptions of
# operators, configure C++ source that encodes the XML as a string.
# The resulting files are placed in the current binary directory and
# appended to genFiles.
include(EncodeCStringFunctions)
function(smtk_operator_xml opSpecs genFiles)
  foreach (opSpec ${opSpecs})
    get_filename_component(genFileBase "${opSpec}" NAME_WE)
    set(genFile "${CMAKE_CURRENT_BINARY_DIR}/${genFileBase}_xml.h")
    #message("Writing ${genFileBase}_xml.cxx (${opSpec})")
    configureFileAsCVariable("${opSpec}" "${genFile}" "${genFileBase}_xml")
    set(${genFiles} ${${genFiles}} "${genFile}" PARENT_SCOPE)
  endforeach()
endfunction()
