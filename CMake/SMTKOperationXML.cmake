include("${CMAKE_CURRENT_LIST_DIR}/EncodeStringFunctions.cmake")

# Given a list of filenames (opSpecs) containing XML descriptions of
# operations, configure C++ source that encodes the XML as a string.
# The resulting files are placed in the current binary directory and
# appended to genFiles.
function(smtk_operation_xml opSpecs genFiles)
  foreach (opSpec ${opSpecs})
    get_filename_component(genFileBase "${opSpec}" NAME_WE)
    set(genFile "${CMAKE_CURRENT_BINARY_DIR}/${genFileBase}_xml.h")
    #message("Writing ${genFileBase}_xml.cxx (${opSpec})")
    configureFileAsCVariable("${opSpec}" "${genFile}" "${genFileBase}_xml")
    set(${genFiles} ${${genFiles}} "${genFile}" PARENT_SCOPE)
  endforeach()
endfunction()

# Given a list of filenames (opSpecs) containing XML descriptions of
# operations, configure Python source that encodes the XML as a string.
# The resulting files are placed in the current binary directory and
# appended to genFiles.
function(smtk_pyoperation_xml opSpecs genFiles)
  foreach (opSpec ${opSpecs})
    get_filename_component(genFileBase "${opSpec}" NAME_WE)
    set(genFile "${CMAKE_CURRENT_BINARY_DIR}/${genFileBase}_xml.py")
    #message("Writing ${genFileBase}_xml.cxx (${opSpec})")
    configureFileAsPyVariable("${opSpec}" "${genFile}" "${genFileBase}_xml")
    set(${genFiles} ${${genFiles}} "${genFile}" PARENT_SCOPE)
  endforeach()
endfunction()
