Attribute update manager
------------------------

The attribute system now has an update manager to aid
you in migrating resources from one schema version
(i.e., template) to another.
See the :ref:`smtk-updaters` update factory documentation
for the basic pattern used to register handlers for
items, attributes, or even whole resource types.

As part of this change, each attribute resource now has
a ``templateType()`` and a ``templateVersion()`` method
to identify the schema from which it is derived.
The values are provided by the SimBuilder Template (SBT) file
used as the prototype for the resource.
Workflow designers should update template files with
a template type and version in order to support future
migration.
