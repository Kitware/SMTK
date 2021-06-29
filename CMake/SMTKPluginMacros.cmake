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

  set(_smtk_plugin_interfaces "")
  set(_smtk_plugin_sources "")
  string(REPLACE ";" "\n  , " _smtk_plugin_managers "${_smtk_plugin_MANAGERS}")

  if (DEFINED _smtk_plugin_REGISTRAR)
    if (NOT DEFINED _smtk_plugin_REGISTRAR_HEADER)
      string(REPLACE "::" "/" _smtk_plugin_header_path "${_smtk_plugin_REGISTRAR}")
      set(_smtk_plugin_REGISTRAR_HEADER "${_smtk_plugin_header_path}.h")
    endif ()

    set(_smtk_plugin_autostart_name "${_smtk_plugin_name}")
    configure_file(
      "${_smtk_cmake_dir}/pqSMTKAutoStart.h.in"
      "${CMAKE_CURRENT_BINARY_DIR}/pqSMTKAutoStart${_smtk_plugin_autostart_name}.h"
      @ONLY)
    configure_file(
      "${_smtk_cmake_dir}/pqSMTKAutoStart.cxx.in"
      "${CMAKE_CURRENT_BINARY_DIR}/pqSMTKAutoStart${_smtk_plugin_autostart_name}.cxx"
      @ONLY)
    paraview_plugin_add_auto_start(
      CLASS_NAME "pqSMTKAutoStart${_smtk_plugin_autostart_name}"
      INTERFACES _smtk_plugin_autostart_interface
      SOURCES _smtk_plugin_autostart_sources)
    list(APPEND _smtk_plugin_interfaces
      ${_smtk_plugin_autostart_interface})
    list(APPEND _smtk_plugin_sources
      "${CMAKE_CURRENT_BINARY_DIR}/pqSMTKAutoStart${_smtk_plugin_autostart_name}.h"
      "${CMAKE_CURRENT_BINARY_DIR}/pqSMTKAutoStart${_smtk_plugin_autostart_name}.cxx"
      ${_smtk_plugin_autostart_sources})
  endif ()
  if (DEFINED _smtk_plugin_REGISTRARS)
    # Additional registrars, must have a unique generated filename.
    foreach (_smtk_plugin_REGISTRAR IN LISTS _smtk_plugin_REGISTRARS)
      string(REPLACE "::" "/" _smtk_plugin_header_path "${_smtk_plugin_REGISTRAR}")
      string(REPLACE "::" "_" _smtk_plugin_header_name "${_smtk_plugin_REGISTRAR}")
      set(_smtk_plugin_REGISTRAR_HEADER "${_smtk_plugin_header_path}.h")

      set(_smtk_plugin_autostart_name "${_smtk_plugin_header_name}")
      configure_file(
        "${_smtk_cmake_dir}/pqSMTKAutoStart.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/pqSMTKAutoStart${_smtk_plugin_autostart_name}.h"
        @ONLY)
      configure_file(
        "${_smtk_cmake_dir}/pqSMTKAutoStart.cxx.in"
        "${CMAKE_CURRENT_BINARY_DIR}/pqSMTKAutoStart${_smtk_plugin_autostart_name}.cxx"
        @ONLY)
      paraview_plugin_add_auto_start(
        CLASS_NAME "pqSMTKAutoStart${_smtk_plugin_autostart_name}"
        INTERFACES _smtk_plugin_autostart_interface
        SOURCES _smtk_plugin_autostart_sources)
      list(APPEND _smtk_plugin_interfaces
        ${_smtk_plugin_autostart_interface})
      list(APPEND _smtk_plugin_sources
        "${CMAKE_CURRENT_BINARY_DIR}/pqSMTKAutoStart${_smtk_plugin_autostart_name}.h"
        "${CMAKE_CURRENT_BINARY_DIR}/pqSMTKAutoStart${_smtk_plugin_autostart_name}.cxx"
        ${_smtk_plugin_autostart_sources})
    endforeach ()
  endif ()

  # FIXME: This shouldn't really be necessary. Instead, SMTK should have its
  # own lightweight plugin macro setup like ParaView does. SMTK can then
  # provide convenience macros to create a ParaView plugin for the SMTK plugin
  # as well.
  paraview_add_plugin("${_smtk_plugin_name}"
    SOURCES ${_smtk_plugin_sources}
    UI_INTERFACES ${_smtk_plugin_interfaces}
    ${_smtk_plugin_PARAVIEW_PLUGIN_ARGS})
  target_link_libraries("${_smtk_plugin_name}"
    PRIVATE
      smtkCore)
endfunction ()
