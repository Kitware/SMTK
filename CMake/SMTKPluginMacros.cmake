#=========================================================================
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=========================================================================

# An internal cache variable for aggregating smtk plugin targets
set(SMTK_PLUGINS "" CACHE INTERNAL "")

# create a plugin
#  REGISTRAR is used to register the plugin
#  REGISTRAR_HEADER is the include file for the registrar (if unset, a file name
#  is inferred from the REGISTRAR value)
#  MANAGERS is a list of managers to which the plugin can register
#  LIBRARIES is a list of libraries against which the plugin must link
#  LIBRARIES_PRIVATE is a list of libraries against which the plugin must
#  privately link
#
#  All other arguments are forwarded to add_paraview_plugin()
#
# add_smtk_plugin(Name Version
#     [REGISTRAR registrar]
#     [REGISTRAR_HEADER headerfile]
#     [MANAGERS list of managers used]
#     [LIBRARIES list of required libraries]
#     [LIBRARIES list of required private libraries]
#  )
function(add_smtk_plugin SMTK_PLUGIN_NAME SMTK_PLUGIN_VERSION)
  set(options)
  set(oneValueArgs REGISTRAR REGISTRAR_HEADER)
  set(multiValueArgs MANAGERS LIBRARIES LIBRARIES_PRIVATE)
  cmake_parse_arguments(SMTK_PLUGIN
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN}
    )

  set_property(GLOBAL APPEND PROPERTY SMTK_PLUGINS "${SMTK_PLUGIN_NAME}")

  string(REPLACE ";" "," SMTK_PLUGIN_MANAGERS_CS "${SMTK_PLUGIN_MANAGERS}")

  if (NOT SMTK_PLUGIN_REGISTRAR_HEADER)
    string(REPLACE "::" "/" tmp "${SMTK_PLUGIN_REGISTRAR}")
    set (SMTK_PLUGIN_REGISTRAR_HEADER "#include \"${tmp}.h\"")
  else()
    set (SMTK_PLUGIN_REGISTRAR_HEADER "#include \"${SMTK_PLUGIN_REGISTRAR_HEADER}\"")
  endif ()

  configure_file(${smtk_cmake_dir}/serverSource.cxx.in
    ${CMAKE_CURRENT_BINARY_DIR}/serverSource.cxx @ONLY)

  find_package(ParaView)
  include(${PARAVIEW_USE_FILE})
  include (ParaViewPlugins)

  # We need to add the current value of VTK_MODULES_DIR to the module path
  # so that when the plugins are built all the modules can be found. Otherwise,
  # modules that aren't loaded as direct dependencies of CMB modules will
  # not be found.
  list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

  add_paraview_plugin(${SMTK_PLUGIN_NAME} ${SMTK_PLUGIN_VERSION}
    SERVER_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/serverSource.cxx
    ${SMTK_PLUGIN_UNPARSED_ARGUMENTS}
    )

  target_link_libraries(${SMTK_PLUGIN_NAME}
    LINK_PUBLIC
      ${SMTK_PLUGIN_LIBRARIES}
      smtkPluginSupport
      vtkPVServerManagerApplication
    LINK_PRIVATE
      vtkPVServerManagerApplicationCS
      ${SMTK_PLUGIN_LIBRARIES_PRIVATE}
)

endfunction(add_smtk_plugin)
