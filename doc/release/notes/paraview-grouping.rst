ParaView Group and Ungroup Menu Items
-------------------------------------

SMTK now includes a new plugin, ``smtkPQGroupingPlugin``, which
adds menu items to ParaView's "Edit" menu named "Group" and "Ungroup."
These menu items are used to create and remove groups for all
resources which have registered operations to the
:smtk:`smtk::operation::GroupingGroup` and
:smtk:`smtk::operation::UngroupingGroup`, respectively.
The menu items have shortcuts of ``Ctrl+G`` and ``Shift+Ctrl+G``,
respectively.

Note that currently only the markup resource provides operations
to the grouping/ungrouping groups.
