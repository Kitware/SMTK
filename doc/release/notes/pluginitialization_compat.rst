Plugin initialization based on ParaView version
-----------------------------------------------

Allow SMTK to create plugins for older versions of ParaView (e.g. v5.9) that
rely on Qt interfaces to invoke initialziation. These changes should not
require any changes to existing plugins using the `smtk_add_plugin` interface.
SMTK plugins that manually implement ParaView plugins via the
`paraview_add_plugin` CMake API should switch to using the SMTK wrapper for
creating SMTK plugins.


Generated Plugin Source:

* Generated sources for older versions or ParaView use an Autostart plugin
and are named using the scheme `pq@PluginName@AutoStart.{h,cxx}`. This will
include the Qt interfaces required for the ParavView Autostart plugin.

* Generated sources for newer versions of ParaView use an initializer
function, similar to VTK, and are named using the scheme
`smtkPluginInitializer@PluginName@.{h,cxx}`. This includes a function
that is namespace'd `smtk::plugin::init::@PluginName@` which is called by
ParaView.
