function(encodeStringAsPyVariable rawString encodedVarName stringOut)
  string(CONFIGURE "\ndescription = '''\n${${rawString}}\n'''\n" pyString)
  set(${stringOut} "${pyString}" PARENT_SCOPE)

endfunction()

function(configureStringAsPyVariable rawString dstFileName encodedVarName)

    encodeStringAsPyVariable(fileContents ${encodedVarName} encodedContents)
    if (EXISTS ${dstFileName})
      file(READ ${dstFileName} already)
    endif()
    #message("Writing ${dstFileName}")
    if (NOT "${encodedContents}" STREQUAL "${already}")
      file(WRITE ${dstFileName} "${encodedContents}")
    endif()

endfunction()

function(configureFileAsPyVariable srcFileName dstFileName encodedVarName)

  if (EXISTS ${srcFileName})
    file(READ ${srcFileName} fileContents)
    configureStringAsPyVariable(fileContents ${dstFileName} ${encodedVarName})
  else()
    file(REMOVE ${dstFileName})
  endif()

endfunction()

# Example:
#   configureFileAsPyVariable("foo.xml" "bar.py" "operatorSpec")
# would read "foo.xml" and write the assigment:
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
