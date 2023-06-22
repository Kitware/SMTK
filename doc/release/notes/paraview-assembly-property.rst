ParaView extensions
-------------------

Representations have an active assembly
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SMTK's custom ParaView representation now provides a string property
named ``Assembly``; this is required by new versions of ParaView so
that block selections (such as performed on context-menu clicks) will
work. On partitioned-dataset assemblies, the assembly name indicates
which (of possibly several) assembly hierarchy shold be used to identify
the selected blocks. For SMTK (currently a multiblock dataset), this
serves no purpose other than to present a crash when no such property
exists.
