More path helper functions
--------------------------

The :smtk:`smtk::common::Paths` class now provides two additional
helper functions:

+ ``areEquivalent()`` to test whether two paths resolve to the
  same location on the filesystem; and
+ ``canonical()`` to return the absolute, canonical (free of symbolic
  links, ``.``, and ``..``) version of a path on the filesystem.
