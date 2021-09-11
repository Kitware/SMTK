Resource subclasses and the move constructor
--------------------------------------------

Because smtk's :smtk:`smtk::common::DerivedFrom` template, used when
inheriting Resource, declares a move constructor with the ``noexcept``
attribute, all subclasses of Resource must explicitly declare a
move constructor with the ``noexcept`` attribute. Modern versions of
clang are strict about checking this.
