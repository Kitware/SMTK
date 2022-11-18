New Operation Groups for Grouping and Ungrouping
------------------------------------------------

The operation subsystem of SMTK now has
a :smtk:`smtk::operation::GroupingGroup` and
a :smtk:`smtk::operation::UngroupingGroup`.
Operations registered to these groups are expected to
associate to group members or groups, respectively.

These groups will be used by ParaView extensions
in the future to create and destroy groups visually
rather than via the operation toolbox.
