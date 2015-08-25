# This will add arguments not found in ${parameter} to the end.  It
# does not attempt to remove duplicate arguments already existing in
# ${parameter}.

MACRO(FORCE_ADD_FLAGS parameter)
  # Create a separated list of the arguments to loop over
  SET(p_list ${${parameter}})
  SEPARATE_ARGUMENTS(p_list)
  # Make a copy of the current arguments in ${parameter}
  SET(new_parameter ${${parameter}})
  # Now loop over each required argument and see if it is in our
  # current list of arguments.
  FOREACH(required_arg ${ARGN})
    # This helps when we get arguments to the function that are
    # grouped as a string:
    #
    # ["-O3 -g"]  instead of [-O3 -g]
    SET(TMP ${required_arg}) #elsewise the Seperate command doesn't work)
    SEPARATE_ARGUMENTS(TMP)
    FOREACH(option ${TMP})
      # Look for the required argument in our list of existing arguments
      SET(found FALSE)
      FOREACH(p_arg ${p_list})
        IF (${p_arg} STREQUAL ${option})
          SET(found TRUE)
        ENDIF (${p_arg} STREQUAL ${option})
      ENDFOREACH(p_arg)
      IF(NOT found)
        # The required argument wasn't found, so we need to add it in.
        SET(new_parameter "${new_parameter} ${option}")
      ENDIF(NOT found)
    ENDFOREACH(option ${TMP})
  ENDFOREACH(required_arg ${ARGN})
  SET(${parameter} ${new_parameter} CACHE STRING "" FORCE)
ENDMACRO(FORCE_ADD_FLAGS)

include(CheckCXXCompilerFlag)
#
# Tests whether the cxx compiler understands a flag.
# If so, add it to 'variable'.
#
# Note: Modified from Deal.II configuration
#
# Usage:
#     ENABLE_IF_SUPPORTED(variable flag)
#
MACRO(ENABLE_IF_SUPPORTED _variable _flag)
  #
  # Clang is too conservative when reporting unsupported compiler flags.
  # Therefore, we promote all warnings for an unsupported compiler flag to
  # actual errors with the -Werror switch:
  #
  IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    SET(_werror_string "-Werror ")
  ELSE()
    SET(_werror_string "")
  ENDIF()

  STRING(STRIP "${_flag}" _flag_stripped)
  IF(NOT "${_flag_stripped}" STREQUAL "")
    STRING(REGEX REPLACE "^-" "" _flag_name "${_flag_stripped}")
    STRING(REPLACE "," "" _flag_name "${_flag_name}")
    STRING(REPLACE "-" "_" _flag_name "${_flag_name}")
    STRING(REPLACE "+" "_" _flag_name "${_flag_name}")
    CHECK_CXX_COMPILER_FLAG(
      "${_werror_string}${_flag_stripped}"
      MOAB_HAVE_FLAG_${_flag_name}
      )
    IF(MOAB_HAVE_FLAG_${_flag_name})
      FORCE_ADD_FLAGS(${_variable} "${_flag_stripped}")
      STRING(STRIP "${${_variable}}" ${_variable})
    ENDIF()
  ENDIF()
ENDMACRO()


