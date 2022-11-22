# Given a string representing an XML file, extract all of the include items and
# replace them with the contents of the file being included.
function(expandXMLString rawString expandedString includeDirectories)

  set(includedFiles)
  # First, grab all of the xml elements describing included files
  string(REGEX MATCHALL "<include[^/]* href=\"[^\"]+\"[^/]*/>" includes ${${rawString}})

  while (includes)
    # For each xml element...
    foreach (include IN LISTS includes)

      # ...extract the attribute with the include file name...
      string(REGEX MATCH "href=\"[^\"]+\"" tmp ${include})

      # ...and extract the substring that contains the bare file name.
      string(LENGTH ${tmp} len)
      math(EXPR newlen "${len} - 7")
      string(SUBSTRING ${tmp} 6 ${newlen} incFileName)

      # Locate the included file so we can expand it. Check the list of include
      # directories, but do not use find_file() as that caches results.
      foreach (includeDir IN LISTS includeDirectories)
        string(CONCAT absoluteIncFileName ${includeDir} "/" ${incFileName})
        if (EXISTS ${absoluteIncFileName})
          set(incFileName ${absoluteIncFileName})
          break()
        endif()
      endforeach()
      find_file(incFileName NAMES ${incFileName} PATHS ${includeDirectories} NO_DEFAULT_PATH)

      list(FIND includedFiles ${incFileName} index)

      if (NOT ${index} EQUAL -1)
        message(WARNING "File \"${incFileName}\" has cyclic dependencies.")
        continue()
      endif ()

      # If we cannot locate the include file, leave the include directive in the
      # xml description.
      if (NOT EXISTS ${incFileName})
        message(WARNING "Cannot locate file \"${incFileName}\".")
        continue()
      endif()

      # Read the dependency file
      file(READ ${incFileName} incFileContents)

      # Add the file name to the list of included files
      list(APPEND includedFiles ${incFileName})

      # Replace the include element with the contents of the included file.
      string(REPLACE ${include} ${incFileContents} ${rawString} "${${rawString}}")
    endforeach()

    # search for more includes, from the new content.
    string(REGEX MATCHALL "<include[^/]* href=\"[^\"]+\"[^/]*/>" includes ${${rawString}})

  endwhile()

  set(${expandedString} "${${rawString}}" PARENT_SCOPE)

endfunction()

function(encodeStringAsCVariable rawString encodedVarName stringOut includeDirectories)

  if ("${includeDirectories}" STREQUAL "")
    set(expandedString "${${rawString}}")
  else()
    expandXMLString(${rawString} expandedString "${includeDirectories}")
  endif()

  # Use a C++11 raw string literal with an unlikely guard word to prevent
  # the need for escaping quotes.
  set(${stringOut} "static const char ${encodedVarName}[] = R\"v0g0nPoetry(${expandedString})v0g0nPoetry\";\n" PARENT_SCOPE)

endfunction()

function(configureStringAsCVariable rawString dstFileName encodedVarName includeDirectories)

  encodeStringAsCVariable(${rawString} ${encodedVarName} encodedContents "${includeDirectories}")
  if (EXISTS ${dstFileName})
    file(READ ${dstFileName} already)
  endif()
  if (NOT "${encodedContents}" STREQUAL "${already}")
    file(WRITE ${dstFileName} "${encodedContents}")
  endif()

endfunction()

function(configureFileAsCVariable srcFileName dstFileName encodedVarName includeDirectories)

  if (EXISTS ${srcFileName})
    file(READ ${srcFileName} fileContents)
    configureStringAsCVariable(fileContents ${dstFileName} ${encodedVarName} "${includeDirectories}")
  else()
    file(REMOVE ${dstFileName})
  endif()

endfunction()

# Example:
#   configureFileAsCVariable("foo.xml" "bar.cxx" "operatorSpec")
# would read "foo.xml", escape it as a C string, and write the
# assignment:
#   static const char operatorSpec[] = "...";
# into bar.cxx (where "..." is the encoded contents of "foo.xml").
#
# When srcFileName does not exist, then dstFileName will be
# removed so that missing files are more easily spotted.
# Note that since this configuration is done when CMake runs
# you must re-run CMake in order to regenerate the file.
#
# The dstFileName file will not be overwritten unless the encoded
# string or variable name differs, in order to avoid recompiling
# dependent files unneccessarily.

function(encodeStringAsPyVariable rawString encodedVarName stringOut includeDirectories)

  expandXMLString(${rawString} expandedString "${includeDirectories}")

  string(CONFIGURE "\ndescription = '''\n${expandedString}\n'''\n" pyString)
  set(${stringOut} "${pyString}" PARENT_SCOPE)

endfunction()

function(configureStringAsPyVariable rawString dstFileName encodedVarName includeDirectories)

    encodeStringAsPyVariable(${rawString} ${encodedVarName} encodedContents "${includeDirectories}")
    if (EXISTS ${dstFileName})
      file(READ ${dstFileName} already)
    endif()
    if (NOT "${encodedContents}" STREQUAL "${already}")
      file(WRITE ${dstFileName} "${encodedContents}")
    endif()

endfunction()

function(configureFileAsPyVariable srcFileName dstFileName encodedVarName includeDirectories)

  if (EXISTS ${srcFileName})
    file(READ ${srcFileName} fileContents)
    configureStringAsPyVariable(fileContents ${dstFileName} ${encodedVarName} "${includeDirectories}")
  else()
    file(REMOVE ${dstFileName})
  endif()

endfunction()

# Example:
#   configureFileAsPyVariable("foo.xml" "bar.py" "operatorSpec")
# would read "foo.xml" and write the assignment:
#   operatorSpec = '''
#   ...
#   '''
# into bar.py (where "..." is the encoded contents of "foo.xml").
#
# When srcFileName does not exist, then dstFileName will be
# removed so that missing files are more easily spotted.
# Note that since this configuration is done when CMake runs
# you must re-run CMake in order to regenerate the file.
#
# The dstFileName file will not be overwritten unless the encoded
# string or variable name differs, in order to avoid recompiling
# dependent files unneccessarily.
