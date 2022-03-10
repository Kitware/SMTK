Plugins
========

SMTK plugins are extensions of ParaView plugins that allow for the
definition of SMTK managers and the automatic registration of
components to these managers. They are created using the CMake
function "add_smtk_plugin", which requires the developer to explicitly list
a registration class known as a "Registrar" and a list of SMTK manager types
to which the plugin registers. SMTK plugins can be introduced to a
ParaView-based application in several ways. The consuming project can

1) list the plugins in a configuration file that is subsequently read at
runtime, deferring the inclusion of plugins to the application's runtime. This
approach requires plugins to reside in certain locations that the application
is expected to look, but facilitates the presentation of a plugin to the user
without automatically loading the plugin. For this approach, a consuming
project can call "generate_smtk_plugin_config_file" to convert the list of
smtk plugin targets (which can be a part of the project or imported from
another project) described by the global property "SMTK_PLUGINS" into a
configuration file. The consuming project can also

2) directly link plugins into the application. This approach pushes the
requirement of locating plugins to be a build-time dependency, which can be
advantageous for packaging. Plugins that are directly linked to an application
cannot be disabled, however (i.e. the target property ENABLED_BY_DEFAULT is
ignored, as it is true for all plugins). To use this approach, a consuming
project can call "generate_smtk_plugin_library" to to use the list of smtk
plugin targets (which can be a part of the project or imported from another
project) described by the global property "SMTK_PLUGINS" to generate a library
against which the application can link to directly incorporate the associated
plugins.

Managers are introduced to SMTK by registering them with the instance
of `smtk::common::Managers`. Upon registration, the singleton instance
of `smtk::plugin::Manager` can be used to register additional plugins
to the newly created manager. See `smtk/resource/Registrar.cxx` for an
example of introducing a manager.
