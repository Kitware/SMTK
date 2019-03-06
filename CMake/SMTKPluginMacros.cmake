#=========================================================================
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=========================================================================

# SMTK plugins are extensions of ParaView plugins that allow for the automatic
# registration of components to SMTK managers. They are created using the
# function "add_smtk_plugin", which requires the developer to explicitly list
# a registration class "known as a Registrar" and a list of SMTK manager types
# to which the plugin registers. SMTK plugins can be introduced to a
# ParaView-based application in several ways. The consuming project can
#
# 1) list the plugins in a configuration file that is subsequently read at
# runtime, deferring the inclusion of plugins to the application's runtime. This
# approach requires plugins to reside in certain locations that the application
# is expected to look, but facilitates the presentation of a plugin to the user
# without automatically loading the plugin. For this approach, a consuming
# project can call "generate_smtk_plugin_config_file" to convert the list of
# smtk plugin targets (which can be a part of the project or imported from
# another project) described by the global property "SMTK_PLUGINS" into a
# configuration file. The consuming project can also
#
# 2) directly link plugins into the application. This approach pushes the
# requirement of locating plugins to be a build-time dependency, which can be
# advantageous for packaging. Plugins that are directly linked to an application
# cannot be disabled, however (i.e. the target property ENABLED_BY_DEFAULT is
# ignored, as it is true for all plugins). To use this approach, a consuming
# project can call "generate_smtk_plugin_library" to to use the list of smtk
# plugin targets (which can be a part of the project or imported from another
# project) described by the global property "SMTK_PLUGINS" to generate a library
# against which the application can link to directly incorporate the associated
# plugins.

define_property(GLOBAL PROPERTY SMTK_PLUGINS
  BRIEF_DOCS "Global property for aggregating smtk plugin targets"
  FULL_DOCS "Global property for aggregating smtk plugin targets")

define_property(TARGET PROPERTY ENABLED_BY_DEFAULT
  BRIEF_DOCS "Option to enable plugin by default"
  FULL_DOCS "Option to enable plugin by default")

# Enable an SMTK plugin by default
#
# enable_smtk_plugin_by_default(target choice)
function(enable_smtk_plugin_by_default target choice)

  # Append a property to the plugin target that conveys whether or not the
  # plugin should be loaded by default
  set_property(TARGET ${target} PROPERTY ENABLED_BY_DEFAULT ${choice})

  # Mark the property for export (so consuming libraries can access it)
  set_property(TARGET ${target} APPEND PROPERTY EXPORT_PROPERTIES ENABLED_BY_DEFAULT)

endfunction(enable_smtk_plugin_by_default)

# create a plugin
#  ENABLED_BY_DEFAULT is an option to indicate whether or not the plugin should
#    be loaded by default. This only applies when plugins are loaded via a
#    plugin list at runtime (as opposed to directly linking to plugins).
#  REGISTRAR is used to register the plugin
#  REGISTRAR_HEADER is the include file for the registrar (if unset, a file name
#    is inferred from the REGISTRAR value)
#  MANAGERS is a list of managers to which the plugin can register
#  LIBRARIES is a list of libraries against which the plugin must link
#  LIBRARIES_PRIVATE is a list of libraries against which the plugin must
#    privately link
#
#  All other arguments are forwarded to add_paraview_plugin()
#
# add_smtk_plugin(Name Version
#     [ENABLED_BY_DEFAULT]
#     [REGISTRAR registrar]
#     [REGISTRAR_HEADER headerfile]
#     [MANAGERS list of managers used]
#     [LIBRARIES list of required libraries]
#     [LIBRARIES list of required private libraries]
#  )
function(add_smtk_plugin SMTK_PLUGIN_NAME SMTK_PLUGIN_VERSION)
  set(options ENABLED_BY_DEFAULT)
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

  enable_smtk_plugin_by_default(${SMTK_PLUGIN_NAME} ${SMTK_PLUGIN_ENABLED_BY_DEFAULT})

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

# create a plugin config file including all of the plugins generated by this
# project.
#  PLUGIN_CONFIG_FILE_NAME is the name of the file to create
#  RELATIVE_DIRECTORY is a directory relative to which plugin locations should
#    be described.
function(generate_smtk_plugin_config_file PLUGIN_CONFIG_FILE_NAME)
  set(options)
  set(oneValueArgs RELATIVE_DIRECTORY)
  set(multiValueArgs)
  cmake_parse_arguments(SMTK_PLUGIN_CONFIG
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN}
    )

  set(plugins_file ${PLUGIN_CONFIG_FILE_NAME})

  set(SMTK_PLUGINS_FILE_CONTENTS)
  foreach (plugin IN LISTS SMTK_PLUGINS)
    get_property(${plugin}_location TARGET ${plugin} PROPERTY LOCATION)
    if (SMTK_PLUGIN_CONFIG_RELATIVE_DIRECTORY)
      file(RELATIVE_PATH ${plugin}_location
        ${SMTK_PLUGIN_CONFIG_RELATIVE_DIRECTORY} ${${plugin}_location})
    endif ()
    get_property(${plugin}_enabled_by_default TARGET ${plugin} PROPERTY ENABLED_BY_DEFAULT)
    set(${plugin}_enabled_by_default_val 0)
    if (${plugin}_enabled_by_default)
      set(${plugin}_enabled_by_default_val 1)
    endif ()
    string(APPEND SMTK_PLUGINS_FILE_CONTENTS
      "  <"
      "Plugin name=\"${plugin}\" "
      "filename=\"${${plugin}_location}\" "
      "auto_load=\"${${plugin}_enabled_by_default_val}\" "
      "/>\n")
  endforeach ()

  configure_file(${smtk_cmake_dir}/plugins.xml.in ${plugins_file} @ONLY)

endfunction(generate_smtk_plugin_config_file)

# create a library that directly links smtk plugins into a consuming
# application. The function creates a library target ${PLUGIN_LIBRARY_TARGET}
# and two header files (defined at parent scope in the list
# ${${PLUGIN_LIBRARY_TARGET}_HEADERS}). All targets contained in the
# ${SMTK_PLUGINS} list will be linked into the target library, and these plugins
# can be loaded by a consuming application by including the generated header
# file Initialize${PLUGIN_LIBRARY_TARGET}.h and by calling the generated methods
# smtk::extension::paraview::initialize${PLUGIN_LIBRARY_TARGET}() and
# smtk::extension::paraview::load${PLUGIN_LIBRARY_TARGET}().
function(generate_smtk_plugin_library PLUGIN_LIBRARY_TARGET)
  include(${PARAVIEW_USE_FILE})
  include(ParaViewPlugins)

  # We need to add the current value of VTK_MODULES_DIR to the module path
  # so that when the plugins are built all the modules can be found. Otherwise,
  # modules that aren't loaded as direct dependencies of CMB modules will
  # not be found.
  list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")

  # Construct fields to populate the generated source files for the plugin
  # library.
  foreach (name IN LISTS SMTK_PLUGINS)
    set(SMTK_PLUGIN_IMPORT_INIT "${SMTK_PLUGIN_IMPORT_INIT}PV_PLUGIN_IMPORT_INIT(${name});\n")
    set(SMTK_PLUGIN_IMPORT "${SMTK_PLUGIN_IMPORT}PV_PLUGIN_IMPORT(${name});\n")
    set(SMTK_PLUGIN_QUERY "${SMTK_PLUGIN_QUERY}queryPlugin(${name});\n")
  endforeach()

  # Generate a unique export symbol for the plugin library.
  string(TOUPPER ${PLUGIN_LIBRARY_TARGET} SMTK_PLUGIN_LIBRARY_EXPORT)
  string(APPEND SMTK_PLUGIN_LIBRARY_EXPORT "_EXPORT")

  # Generate the header file that declares the two methods defined in the plugin
  # library.
  configure_file(${smtk_cmake_dir}/InitializePlugins.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/Initialize${PLUGIN_LIBRARY_TARGET}.h @ONLY)

  # Generate the source file that implements the abovementioned methods.
  configure_file(${smtk_cmake_dir}/InitializePlugins.cxx.in
    ${CMAKE_CURRENT_BINARY_DIR}/Initialize${PLUGIN_LIBRARY_TARGET}.cxx @ONLY)

  # Include the components from ParaView necessary for directly linking plugins
  # into an application.
  vtk_module_dep_includes(vtkPVClientServerCoreCore)
  include_directories(${vtkPVClientServerCoreCore_INCLUDE_DIRS}
    ${vtkPVClientServerCoreCore_DEPENDS_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR})

  # Construct the library.
  add_library(${PLUGIN_LIBRARY_TARGET}
    ${CMAKE_CURRENT_BINARY_DIR}/Initialize${PLUGIN_LIBRARY_TARGET}.cxx)

  # During the build phase, include the binary directory that contains the
  # generated header file.
  target_include_directories(${PLUGIN_LIBRARY_TARGET}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    )

  # Link against all of the smtk plugins.
  target_link_libraries(${PLUGIN_LIBRARY_TARGET}
    LINK_PRIVATE ${SMTK_PLUGINS} vtkPVClientServerCoreCore)

  # Generate an export header using the symbol defined in
  # ${SMTK_PLUGIN_LIBRARY_EXPORT}.
  include(GenerateExportHeader)
  generate_export_header(${PLUGIN_LIBRARY_TARGET}
    EXPORT_MACRO_NAME ${SMTK_PLUGIN_LIBRARY_EXPORT}
    EXPORT_FILE_NAME ${PLUGIN_LIBRARY_TARGET}Export.h)

  # Construct a list of generated headers for the plugin library that is
  # accessible at parent scope. That way, consuming applications can install
  # these header files where appropriate.
  set(${PLUGIN_LIBRARY_TARGET}_HEADERS
    ${CMAKE_CURRENT_BINARY_DIR}/Initialize${PLUGIN_LIBRARY_TARGET}.h
    ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_LIBRARY_TARGET}Export.h
    PARENT_SCOPE)

endfunction(generate_smtk_plugin_library)
