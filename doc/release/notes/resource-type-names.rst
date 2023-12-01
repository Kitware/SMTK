Resource system
---------------

Resource Manager
~~~~~~~~~~~~~~~~

The :smtk:`smtk::resource::Manager` class now provides an ``objectTypeLabels()``
method returning a map that registrars can use to register human-readable
(and application-specific) strings given an object's type-name.
This method is intended to map fully-qualified type-names for *any* class to
labels that are descriptive to users, not just subclasses of
:smtk:`smtk::resource::PersistentObject`.
Labels should be as short as possible while remaining descriptive;
applications should not expect labels to be sentences or paragraphs of text.

If you use the ``smtkTypeMacroBase()``/``smtkTypeMacro()`` macros,
you can use the virtual ``typeToken()`` method on any object to identify its
class name and search the map returned by ``objectTypeLabels()`` to obtain
a human-readable string.

You can also use the :smtk:`smtk::common::typeName` template to identify a
string for any class and look it up in the map.

Applications should only look names up; registrars should write data to the map.
If existing registrars use a name that is unsuitable for your application,
simply create a registrar in your application whose ``Dependencies`` tuple
lists these registrars and overwrite their strings with ones better for your
application; because your application registrar depends on others, it will
always be invoked last.
