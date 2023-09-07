Attribute System
----------------

Referencing associations via ``itemAtPath()``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can now reference an attribute's associations with the
:smtk:`itemAtPath() <smtk::attribute::Attribute::itemAtPath>`
method by passing in the name assigned to the association-definition rule.
To support this, :smtk:`smtk::attribute::Definition::createLocalAssociationRule`
now accepts a name string. You may also specify the association-rule's name
via XML.

Note that if an item with the same name as the association rule exists, that
item will always be returned instead of the association rule.
You are responsible for ensuring names are unique; SMTK will not prevent
name collisions with the association rule.
