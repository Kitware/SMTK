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
    "_SKIP_DEPENDENCIES;ALLOW_QT_KEYWORDS"
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
  set(_smtk_plugin_extra_includes "")
  set(_smtk_plugin_initializers "")
  set(_smtk_plugin_sources "")
  string(REPLACE ";" "\n  , " _smtk_plugin_managers "${_smtk_plugin_MANAGERS}")

  set(_smtk_plugin_registrars)
  if (DEFINED _smtk_plugin_REGISTRAR)
    list(APPEND _smtk_plugin_registrars ${_smtk_plugin_REGISTRAR})
    if (NOT DEFINED _smtk_plugin_REGISTRAR_HEADER)
      string(REPLACE "::" "/" _smtk_plugin_header_path "${_smtk_plugin_REGISTRAR}")
      set(_smtk_plugin_REGISTRAR_HEADER "${_smtk_plugin_header_path}.h")
    endif ()
    set_property(GLOBAL PROPERTY _smtk_plugin_registrar_header${_smtk_plugin_REGISTRAR} ${_smtk_plugin_REGISTRAR_HEADER})
  endif ()
  if (DEFINED _smtk_plugin_REGISTRARS)
    list(APPEND _smtk_plugin_registrars ${_smtk_plugin_REGISTRARS})
    foreach (_smtk_plugin_REGISTRAR IN LISTS _smtk_plugin_REGISTRARS)
      string(REPLACE "::" "/" _smtk_plugin_header_path "${_smtk_plugin_REGISTRAR}")
      set(_smtk_plugin_REGISTRAR_HEADER "${_smtk_plugin_header_path}.h")
      set_property(GLOBAL PROPERTY "_smtk_plugin_registrar_header${_smtk_plugin_REGISTRAR}" ${_smtk_plugin_REGISTRAR_HEADER})
    endforeach ()
  endif ()

  foreach (_smtk_plugin_REGISTRAR IN LISTS _smtk_plugin_registrars)
    string(REPLACE "::" "_" _smtk_plugin_header_name "${_smtk_plugin_REGISTRAR}")
    get_property(_smtk_plugin_REGISTRAR_HEADER GLOBAL PROPERTY "_smtk_plugin_registrar_header${_smtk_plugin_REGISTRAR}")
    set(_smtk_plugin_autostart_name "${_smtk_plugin_header_name}")
    if (ParaView_VERSION VERSION_GREATER_EQUAL "5.10.0")
      set(_smtk_plugin_init_header
        "${CMAKE_CURRENT_BINARY_DIR}/smtkPluginInitializer${_smtk_plugin_autostart_name}.h")
      configure_file(
        "${_smtk_cmake_dir}/smtkPluginInitializer.h.in"
        "${_smtk_plugin_init_header}"
        @ONLY)
      list(APPEND _smtk_plugin_extra_includes "${_smtk_plugin_init_header}")

      set(_smtk_plugin_init_impl
        "${CMAKE_CURRENT_BINARY_DIR}/smtkPluginInitializer${_smtk_plugin_autostart_name}.cxx")
      configure_file(
        "${_smtk_cmake_dir}/smtkPluginInitializer.cxx.in"
        "${_smtk_plugin_init_impl}"
        @ONLY)
      list(APPEND _smtk_plugin_initializers "smtk::plugin::init::${_smtk_plugin_autostart_name}")

      list(APPEND _smtk_plugin_sources
        "${_smtk_plugin_init_header}"
        "${_smtk_plugin_init_impl}"
      )
    else ()
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
      list(APPEND _smtk_plugin_interfaces ${_smtk_plugin_autostart_interface})
      list(APPEND _smtk_plugin_sources
        "${CMAKE_CURRENT_BINARY_DIR}/pqSMTKAutoStart${_smtk_plugin_autostart_name}.h"
        "${CMAKE_CURRENT_BINARY_DIR}/pqSMTKAutoStart${_smtk_plugin_autostart_name}.cxx"
        ${_smtk_plugin_autostart_sources})
    endif ()
  endforeach ()
  if (NOT "${_smtk_plugin_interfaces}" STREQUAL "")
    list(PREPEND _smtk_plugin_interfaces "UI_INTERFACES")
  endif()
  if (NOT "${_smtk_plugin_initializers}" STREQUAL "")
    list(PREPEND _smtk_plugin_initializers "INITIALIZERS")
  endif()
  if (NOT "${_smtk_plugin_extra_includes}" STREQUAL "")
    list(PREPEND _smtk_plugin_extra_includes "EXTRA_INCLUDES")
  endif()

  # FIXME: This shouldn't really be necessary. Instead, SMTK should have its
  # own lightweight plugin macro setup like ParaView does. SMTK can then
  # provide convenience macros to create a ParaView plugin for the SMTK plugin
  # as well.
  paraview_add_plugin("${_smtk_plugin_name}"
    SOURCES ${_smtk_plugin_sources}
    ${_smtk_plugin_extra_includes}
    ${_smtk_plugin_initializers}
    ${_smtk_plugin_interfaces}
    ${_smtk_plugin_PARAVIEW_PLUGIN_ARGS})
  target_link_libraries("${_smtk_plugin_name}"
    PRIVATE
      smtkCore)
  # only enforce QT_NO_KEYWORDS if the option is not set.
  if (NOT _smtk_plugin_ALLOW_QT_KEYWORDS)
    target_compile_definitions("${_smtk_plugin_name}" PRIVATE QT_NO_KEYWORDS)
  endif()
endfunction ()
