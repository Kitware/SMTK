Added the ability to "Export" ItemBlocks
----------------------------------------

Previously ItemBlocks were file-scope only.  This change extends ItemBlocks so that an ItemBlock defined in one
file can be consumed by another file that includes it. To maintain backward compatibility, an ItemBlock that is to
be exported must include the **Export** XML attribute and be set to *true*.  In order to better organize ItemBlocks,
SMTK now supports the concept of Namespaces.

**Note** Namespaces are only used w/r ItemBlocks and can not be nested.  To indicate an ItemBlock is defined with an specific Namespace NS,
you will need to use the **Namespace** XML attribute.  As a shortcut, if all of the ItemBlocks in a file are to belong to the same Namespace,
you can specify the **Namespace** XML attribute at the **ItemBlocks** level as well.  If no Namespace is specified, SMTK assumes the ItemBlock
belongs to the global Namespace represented by "".
