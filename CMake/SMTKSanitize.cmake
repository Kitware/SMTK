#=========================================================================
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=========================================================================

# This code has been adapted from remus (https://gitlab.kitware.com/cmb/remus)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR
   CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  set(CMAKE_COMPILER_IS_CLANGXX 1)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)

  #Add option for enabling sanitizers
  option(SMTK_ENABLE_SANITIZER "Build with sanitizer support." OFF)
  mark_as_advanced(SMTK_ENABLE_SANITIZER)

  if(SMTK_ENABLE_SANITIZER)
    set(SMTK_SANITIZER "address"
      CACHE STRING "The sanitizer to use")
    mark_as_advanced(SMTK_SANITIZER)

    #We're setting the CXX flags and C flags beacuse they're propagated down
    #independent of build type.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=${SMTK_SANITIZER}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=${SMTK_SANITIZER}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=${SMTK_SANITIZER}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=${SMTK_SANITIZER}")
  endif()
endif()

function (smtk_test_apply_sanitizer)
  # Do nothing if we're not building under a sanitizer.
  if (NOT SMTK_ENABLE_SANITIZER OR NOT SMTK_SANITIZER STREQUAL "address")
    return ()
  endif ()

  # Bail on non-Unix or macOS.
  if (NOT UNIX OR APPLE)
    return ()
  endif ()

  find_library(LIBASAN_LIBRARY NAMES asan libasan.so.8 libasan.so.7 libasan.so.6 libasan.so.5)
  mark_as_advanced(LIBASAN_LIBRARY)

  # Bail if we can't find `libasan`.
  if (NOT LIBASAN_LIBRARY)
    return ()
  endif ()

  set_property(TEST ${ARGN} APPEND
    PROPERTY
      # XXX(cmake-3.22): use `ENVIRONMENT_MODIFICATION`.
      # ENVIRONMENT_MODIFICATION "LD_PRELOAD=path_list_prepend:${LIBASAN_LIBRARY}")
      ENVIRONMENT "LD_PRELOAD=${LIBASAN_LIBRARY}")
endfunction ()
