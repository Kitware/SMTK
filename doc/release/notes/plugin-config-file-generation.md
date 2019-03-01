Add cmake logic to generate a plugin config file

ParaView-derived applications currently ingest plugins in one of two
ways: the plugins are either linked directly into the application, or
they are loaded at runtime. For the latter case, plugin discovery is
performed by reading xml files that describe the plugin's name,
location and whether or not it should be automatically loaded.

When SMTK (and SMTK-derived) plugins are intended to be loaded at
runtime, it is convenient to have the plugin config file generated
using the CMake target properties from the plugins themselves. This
change introduces the exported plugin target property
`ENABLED_BY_DEFAULT` and adds the CMake function
`generate_smtk_plugin_config_file` for ParaView-derived applications
that consume SMTK plugins to generate a plugins file.
