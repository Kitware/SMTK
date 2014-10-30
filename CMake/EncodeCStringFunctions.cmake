function(encodeStringAsCVariable rawString encodedVarName stringOut)

  string(REPLACE "\\" "\\\\" str1 "${${rawString}}")
  string(REPLACE "\"" "\\\"" str2 "${str1}")
  string(REPLACE "\n" "\\n" str3 "${str2}")
  string(CONFIGURE
    "\nstatic const char ${encodedVarName}[] = \"${str3}\";\n\n"
    escaped ESCAPE_QUOTES)
  set(${stringOut} "${escaped}" PARENT_SCOPE)

endfunction()

function(configureFileAsCVariable srcFileName dstFileName encodedVarName)

  if (EXISTS ${srcFileName})
    file(READ ${srcFileName} fileContents)
    encodeStringAsCVariable(fileContents ${encodedVarName} encodedContents)
    if (EXISTS ${dstFileName})
      file(READ ${dstFileName} already)
    endif()
    #message("Writing ${dstFileName}")
    if (NOT "${encodedContents}" STREQUAL "${already}")
      file(WRITE ${dstFileName} "${encodedContents}")
    endif()
  else()
    file(REMOVE ${dstFileName})
  endif()

endfunction()

# Example:
#   configureFileAsCVariable("foo.xml" "bar.cxx" "operatorSpec")
# would read "foo.xml", escape it as a C string, and write the
# assigment:
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
