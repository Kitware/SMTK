function (_smtk_package_append_variables)
  set(_smtk_package_variables)
  foreach (var IN LISTS ARGN)
    if (NOT ${var})
      continue ()
    endif ()

    get_property(type_is_set CACHE "${var}"
      PROPERTY TYPE SET)
    if (type_is_set)
      get_property(type CACHE "${var}"
        PROPERTY TYPE)
    else ()
      set(type UNINITIALIZED)
    endif ()

    string(APPEND _smtk_package_variables
      "if (NOT DEFINED \"${var}\" OR NOT ${var})
  set(\"${var}\" \"${${var}}\" CACHE ${type} \"Third-party helper setting from \${CMAKE_FIND_PACKAGE_NAME}\")
endif ()
")
  endforeach ()

  set(smtk_find_package_code
    "${smtk_find_package_code}${_smtk_package_variables}"
    PARENT_SCOPE)
endfunction ()

set(_smtk_packages
  Boost
  nlohmann_json
  pegtl
  LibArchive
  MOAB
  Qt5
  ParaView
  Python${SMTK_PYTHON_VERSION}
  VTK
  Remus
  ZeroMQ # for Remus
  pybind11)

# Per-package variable forwarding goes here.
set(Boost_find_package_vars
  Boost_INCLUDE_DIR)
set(LibArchive_find_package_vars
  LibArchive_INCLUDE_DIR
  LibArchive_LIBRARY)
set(ZeroMQ_find_package_vars
  ZeroMQ_INCLUDE_DIR
  ZeroMQ_LIBRARY)

set(smtk_find_package_code)
foreach (_smtk_package IN LISTS _smtk_packages)
  _smtk_package_append_variables(
    # Standard CMake `find_package` mechanisms.
    "${_smtk_package}_DIR"
    "${_smtk_package}_ROOT"

    # Per-package custom variables.
    ${${_smtk_package}_find_package_vars})
endforeach ()

file(GENERATE
  OUTPUT  "${smtk_cmake_build_dir}/smtk-find-package-helpers.cmake"
  CONTENT "${smtk_find_package_code}")
