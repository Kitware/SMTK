#=========================================================================
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=========================================================================

# SMTK plugins are extensions of ParaView plugins that allow for the automatic
# registration of components to SMTK managers. They are created using the
# function "smtk_add_plugin", which requires the developer to explicitly list
# a registration class known as a "Registrar" and a list of SMTK manager types
# to which the plugin registers. SMTK plugins can be introduced to a
# ParaView-based application in several ways.

set(_smtk_cmake_dir "${CMAKE_CURRENT_LIST_DIR}")

function (smtk_add_plugin name)
  set(_smtk_plugin_name "${name}")
  cmake_parse_arguments(_smtk_plugin
    "_SKIP_DEPENDENCIES"
    "REGISTRAR;REGISTRAR_HEADER"
    "MANAGERS;REGISTRARS;PARAVIEW_PLUGIN_ARGS"
    ${ARGN})

  if (_smtk_plugin_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR
      "Unparsed arguments for `smtk_add_plugin`: "
      "${_smtk_plugin_UNPARSED_ARGUMENTS}")
  endif ()

  if (NOT DEFINED _smtk_plugin_REGISTRAR AND NOT DEFINED _smtk_plugin_REGISTRARS)
    message(FATAL_ERROR
      "The `REGISTRAR` or `REGISTRARS` argument is required.")
  endif ()

  if (NOT DEFINED _smtk_plugin_MANAGERS)
    message(FATAL_ERROR
      "The `MANAGERS` argument is required.")
  endif ()

  string(TOLOWER "${_smtk_plugin__SKIP_DEPENDENCIES}" _smtk_plugin__SKIP_DEPENDENCIES)

  set(_smtk_plugin_sources "")
  string(REPLACE ";" ", " _smtk_plugin_managers "${_smtk_plugin_MANAGERS}")

  if (DEFINED _smtk_plugin_REGISTRAR)
    if (NOT DEFINED _smtk_plugin_REGISTRAR_HEADER)
      string(REPLACE "::" "/" _smtk_plugin_header_path "${_smtk_plugin_REGISTRAR}")
      set(_smtk_plugin_REGISTRAR_HEADER "${_smtk_plugin_header_path}.h")
    endif ()

    configure_file(
      "${_smtk_cmake_dir}/serverSource.cxx.in"
      "${CMAKE_CURRENT_BINARY_DIR}/serverSource.cxx"
      @ONLY)
    list(APPEND _smtk_plugin_sources "${CMAKE_CURRENT_BINARY_DIR}/serverSource.cxx")
  else ()
    # More than one registrar, must have a unique generated filename.
    foreach(_smtk_plugin_REGISTRAR ${_smtk_plugin_REGISTRARS})
      string(REPLACE "::" "/" _smtk_plugin_header_path "${_smtk_plugin_REGISTRAR}")
      string(REPLACE "::" "_" _smtk_plugin_header_name "${_smtk_plugin_REGISTRAR}")
      set(_smtk_plugin_REGISTRAR_HEADER "${_smtk_plugin_header_path}.h")
      set(_smtk_plugin_filename "${CMAKE_CURRENT_BINARY_DIR}/serverSource_${_smtk_plugin_header_name}.cxx")

      configure_file(
        "${_smtk_cmake_dir}/serverSource.cxx.in"
        "${_smtk_plugin_filename}"
        @ONLY)
      list(APPEND _smtk_plugin_sources "${_smtk_plugin_filename}")
    endforeach(_smtk_plugin_REGISTRAR)
  endif ()

  # FIXME: This shouldn't really be necessary. Instead, SMTK should have its
  # own lightweight plugin macro setup like ParaView does. SMTK can then
  # provide convenience macros to create a ParaView plugin for the SMTK plugin
  # as well.
  paraview_add_plugin("${_smtk_plugin_name}"
    SOURCES ${_smtk_plugin_sources}
    ${_smtk_plugin_PARAVIEW_PLUGIN_ARGS})
  target_link_libraries("${_smtk_plugin_name}"
    PRIVATE
      smtkCore
      smtkPluginSupport)
endfunction ()
