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
#  All other arguments are forwarded on to add_paraview_plugin()
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

  if (SMTK_PLUGINS)
    set(SMTK_PLUGINS "${SMTK_PLUGINS};${SMTK_PLUGIN_NAME}" CACHE INTERNAL "")
  else ()
    set(SMTK_PLUGINS "${SMTK_PLUGIN_NAME}" CACHE INTERNAL "")
  endif ()

  string(REPLACE ";" "," SMTK_PLUGIN_MANAGERS_CS "${SMTK_PLUGIN_MANAGERS}")

  if (NOT SMTK_PLUGIN_REGISTRAR_HEADER)
    string(REPLACE "::" "/" tmp "${SMTK_PLUGIN_REGISTRAR}")
    set (SMTK_PLUGIN_REGISTRAR_HEADER "#include \"${tmp}.h\"")
  else()
    set (SMTK_PLUGIN_REGISTRAR_HEADER "#include \"${SMTK_PLUGIN_REGISTRAR_HEADER}\"")
  endif ()

  configure_file(${SMTK_SOURCE_DIR}/CMake/serverSource.cxx.in
    ${CMAKE_CURRENT_BINARY_DIR}/serverSource.cxx @ONLY)

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
smtk_install_library(${SMTK_PLUGIN_NAME})

endfunction(add_smtk_plugin)

# Once all of the plugins have been processed, the cache variable SMTK_PLUGINS
# will contain all of the plugins to be built by SMTK. The
# generate_plugins_init() function creates the smtkPluginsInit library that
# provides an API for loading smtk plugins at runtime by name.
function(generate_plugins_init)

  write_plugins_init_file(
    ${CMAKE_CURRENT_BINARY_DIR}/smtk/PluginsInit.h
    ${CMAKE_CURRENT_BINARY_DIR}/smtk/PluginsInit.cxx
    ${SMTK_PLUGINS})

  include(${PARAVIEW_USE_FILE})

  vtk_module_dep_includes(vtkPVClientServerCoreCore)
  include_directories(${vtkPVClientServerCoreCore_INCLUDE_DIRS} ${vtkPVClientServerCoreCore_DEPENDS_INCLUDE_DIRS})
  add_library(smtkPluginsInit
    ${CMAKE_CURRENT_BINARY_DIR}/smtk/PluginsInit.h
    ${CMAKE_CURRENT_BINARY_DIR}/smtk/PluginsInit.cxx)
  target_link_libraries(smtkPluginsInit
    LINK_PRIVATE ${SMTK_PLUGINS})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/smtk/PluginsInit.h DESTINATION include/smtk/${SMTK_VERSION}/smtk)
smtk_install_library(smtkPluginsInit)

endfunction(generate_plugins_init)

# Internal function used to generate a header file initializing all plugins that
# can be used by executables to link against the plugins.
function(write_plugins_init_file header source)

  file(WRITE "${header}" "void smtk_plugins_init();\n")

  set(plugins_init "#include \"vtkPVPlugin.h\"\n")
  set(plugins_init "${plugins_init}#include \"vtkPVPluginLoader.h\"\n\n")
  set(plugins_init "${plugins_init}#include \"vtkPVPluginTracker.h\"\n\n")
  set(plugins_init "${plugins_init}#include <string>\n\n")

  # write PV_PLUGIN_IMPORT_INIT calls
  foreach(plugin_name ${ARGN})
    set(plugins_init "${plugins_init}PV_PLUGIN_IMPORT_INIT(${plugin_name});\n")
  endforeach()
  set(plugins_init "${plugins_init}\n")

  set(plugins_init "${plugins_init}static bool smtk_plugins_load(const char* name);\n\n")
  set(plugins_init "${plugins_init}static bool smtk_plugins_search(const char* name);\n\n")
  set(plugins_init "${plugins_init}void smtk_plugins_init()\n{\n")
  set(plugins_init "${plugins_init}  vtkPVPluginLoader::SetStaticPluginLoadFunction(smtk_plugins_load);\n")
  set(plugins_init "${plugins_init}  vtkPVPluginTracker::SetStaticPluginSearchFunction(smtk_plugins_search);\n")
  set(plugins_init "${plugins_init}}\n\n")

  # write callback functions
  set(plugins_init "${plugins_init}static bool smtk_plugins_func(const char* name, bool load);\n\n")
  set(plugins_init "${plugins_init}static bool smtk_plugins_load(const char* name)\n{\n")
  set(plugins_init "${plugins_init}  return smtk_plugins_func(name, true);\n")
  set(plugins_init "${plugins_init}}\n\n")
  set(plugins_init "${plugins_init}static bool smtk_plugins_search(const char* name)\n{\n")
  set(plugins_init "${plugins_init}  return smtk_plugins_func(name, false);\n")
  set(plugins_init "${plugins_init}}\n\n")

  # write PV_PLUGIN_IMPORT calls
  set(plugins_init "${plugins_init}static bool smtk_plugins_func(const char* name, bool load)\n{\n")
  set(plugins_init "${plugins_init}  std::string sname = name;\n\n")
  foreach(plugin_name ${ARGN})
    set(plugins_init "${plugins_init}  if (sname == \"${plugin_name}\")\n")
    set(plugins_init "${plugins_init}    {\n")
    set(plugins_init "${plugins_init}    if (load)\n")
    set(plugins_init "${plugins_init}      {\n")
    set(plugins_init "${plugins_init}      static bool loaded = false;\n")
    set(plugins_init "${plugins_init}      if (!loaded)\n")
    set(plugins_init "${plugins_init}        {\n")
    set(plugins_init "${plugins_init}        loaded = PV_PLUGIN_IMPORT(${plugin_name});\n")
    set(plugins_init "${plugins_init}        }\n")
    set(plugins_init "${plugins_init}      }\n")
    set(plugins_init "${plugins_init}    return true;\n")
    set(plugins_init "${plugins_init}    }\n")
  endforeach()
  set(plugins_init "${plugins_init}  return false;\n")
  set(plugins_init "${plugins_init}}\n")

  file(WRITE "${source}" "${plugins_init}")

endfunction()
