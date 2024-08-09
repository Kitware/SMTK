Graph Resource
--------------

An issue with the :smtk:`smtk::graph::Resource` template's ``clone()`` method
has been identified. When the source resource does not override ``clone()`` and
does not have a resource-manager, the base ``clone()`` implementation will no
longer attempt to create a resource. If you encounter this situation, you are
now expected to override the ``clone()`` method in your subclass and call the
new ``prepareClone()`` method with the resource you create.
