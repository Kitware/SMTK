VTK resource multi-block source changes
---------------------------------------

If an :smtk:`smtk::geometry::Geometry` provider adds the name
of a component to a cache entry (by setting the ``vtkCompositeDataSet::NAME()``
key the cache entry's ``vtkInformation`` object), the
:smtk:`vtkSMTKResourceMultiBlockSource` filter will copy the information
key to its output.

If you are an end user of a ParaView-based application, then the information
panel may now display more helpful information in its block-inspector tree.

If you maintain your own geometry cache for a custom resource type and wish
users to see the name in ParaView's (and/or ModelBuilder's) information panel,
then you should update your geometry object to set the name.
