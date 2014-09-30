##=============================================================================
##
##  Copyright (c) Kitware, Inc.
##  All rights reserved.
##  See LICENSE.txt for details.
##
##  This software is distributed WITHOUT ANY WARRANTY; without even
##  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
##  PURPOSE.  See the above copyright notice for more information.
##
##=============================================================================

## This CMake script checks source files for the appropriate copyright
## statement, which is stored in:
## SMTK_SOURCE_DIR/CMake/CopyrightStatement.txt
## To run this script, execute CMake as follows:

cmake_minimum_required(VERSION 2.8)

set(FILES_TO_CHECK
  *.h
  *.h.in
  *.cxx
  *.py
  )

if (NOT SMTK_SOURCE_DIR)
  message(SEND_ERROR "SMTK_SOURCE_DIR not defined.")
endif()

set(copyright_file ${SMTK_SOURCE_DIR}/CMake/CopyrightStatement.txt)

if (NOT EXISTS ${copyright_file})
  message(SEND_ERROR "Cannot find CopyrightStatement.txt")
endif()

# Escapes ';' characters (list delimiters) and splits the given string into
# a list of its lines without newlines.
function (list_of_lines var string)
  string(REGEX REPLACE ";" "\\\\;" conditioned_string "${string}")
  string(REGEX REPLACE "\n" ";" conditioned_string "${conditioned_string}")
  set(${var} "${conditioned_string}" PARENT_SCOPE)
endfunction()

# Read in copyright statement file.
file(READ ${copyright_file} COPYRIGHT_STATEMENT)

# Remove trailing whitespace and ending lines.  They are sometimes hard to
# see or remove in editors.
string(REGEX REPLACE "[ \t]*\n" "\n" COPYRIGHT_STATEMENT "${COPYRIGHT_STATEMENT}")
string(REGEX REPLACE "\n+$" "" COPYRIGHT_STATEMENT "${COPYRIGHT_STATEMENT}")

# Get a list of lines in the copyright statement.
list_of_lines(COPYRIGHT_LINE_LIST "${COPYRIGHT_STATEMENT}")

# Comment regular expression characters that we want to match literally.
string(REPLACE "." "\\." COPYRIGHT_LINE_LIST "${COPYRIGHT_LINE_LIST}")
string(REPLACE "(" "\\(" COPYRIGHT_LINE_LIST "${COPYRIGHT_LINE_LIST}")
string(REPLACE ")" "\\)" COPYRIGHT_LINE_LIST "${COPYRIGHT_LINE_LIST}")

# Print an error concerning the missing copyright in the given file.
function(missing_copyright filename comment_prefix)
  message("${filename} does not have the appropriate copyright statement:\n")

  # Condition the copyright statement
  string(REPLACE
    "\n"
    "\n${comment_prefix}  "
    comment_copyright
    "${COPYRIGHT_STATEMENT}"
    )
  set(comment_copyright "${comment_prefix}  ${comment_copyright}")
  string(REPLACE
    "\n${comment_prefix}  \n"
    "\n${comment_prefix}\n"
    comment_copyright
    "${comment_copyright}"
    )

  message("${comment_prefix}=============================================================================")
  message("${comment_prefix}")
  message("${comment_copyright}")
  message("${comment_prefix}")
  message("${comment_prefix}=============================================================================\n")
  message(SEND_ERROR
    "Please add the previous statement to the beginning of ${filename}"
    )
endfunction()

# Check the given file for the appropriate copyright statement.
function(check_copyright filename)
  get_filename_component(file_ext "${filename}" EXT)

  if(file_ext STREQUAL ".py")
    set(comment_prefix "#")
  else()
    set(comment_prefix "//")
  endif()

  # Read in the first 2000 characters of the file and split into lines.
  # This is roughly equivalent to the file STRINGS command except that we
  # also escape semicolons (list separators) in the input, which the file
  # STRINGS command does not currently do.
  file(READ "${filename}" header_contents LIMIT 2000)
  list_of_lines(header_lines "${header_contents}")

  # Check each copyright line.
  foreach (copyright_line IN LISTS COPYRIGHT_LINE_LIST)
    set(match)
    # My original algorithm tried to check the order by removing items from
    # header_lines as they were encountered.  Unfortunately, CMake 2.8's
    # list REMOVE_AT command removed the escaping on the ; in one of the
    # header_line's items and cause the compare to fail.
    foreach (header_line IN LISTS header_lines)
      if (copyright_line)
	     string(REGEX MATCH
	            "^${comment_prefix}[ \t]*${copyright_line}[ \t]*$"
	            match
	            "${header_line}"
              )
      else()
        if (NOT header_line)
          set(match TRUE)
        endif()
      endif()

      if(match)
        break()
      endif()

    endforeach()
    if(NOT match)
      message(STATUS "Could not find match for `${copyright_line}'")
      missing_copyright("${filename}" "${comment_prefix}")
    endif()
  endforeach()
endfunction()

foreach (glob_expression ${FILES_TO_CHECK})
  file(GLOB_RECURSE file_list
    RELATIVE "${SMTK_SOURCE_DIR}/smtk"
    "${SMTK_SOURCE_DIR}/smtk/${glob_expression}"
    )
  foreach (file ${file_list})
    message("Checking ${file}")
    check_copyright("${SMTK_SOURCE_DIR}/smtk/${file}")
  endforeach()
endforeach()
