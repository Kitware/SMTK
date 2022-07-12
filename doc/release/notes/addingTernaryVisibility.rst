Added Ternary Visibility Badge Support
--------------------------------------

Created two new badges - one for controlling a phrase's geometric visibility
and another for controlling / displaying the visibility of the phrase's
children, i.e. hierarchy.

The :smtk:`geometric visibility badge <smtk::extension::paraview::appcomponents::GeometricVisibilityBadge>`
is binary and can have the following values:

  * *Visible* if the object's geometry is visible
  * *Invisible* if the object's geometry is invisible

The :smtk:`hierarchical visibility badge <smtk::extension::paraview::appcomponents::HierarchicalVisibilityBadge>`is
ternary and can have the following values:

  * *Visible* if all of its children are visible
  * *Invisible* if all of its children are invisible
  * *Neither* if some of its children are marked *Neither* and/or not all of its children are visible

The user can set all of the phrase's descendants' visibilities by toggling the Hierarchical Badge.

**Note:** The geometric visibility badge of phrase that corresponds to a resource will
effect the ParaView Representation Visibility.

**Note:** The hierarchical visibility badge of an object does not affect the geometric visibility of the object itself.
