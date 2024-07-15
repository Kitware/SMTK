Attribute and Item Views
------------------------

Most of the Qt subsytem is dedicated to providing users with ways
to create, delete, and modify attributes.
There are two Qt-specific subclasses of :smtk:`smtk::view::BaseView`
dedicated to this:

+ :smtk:`smtk::extension::qtInstancedView` is a view dedicated to
  displaying and editing a single, named instance of an attribute.
+ :smtk:`smtk::extension::qtAttributeView` is a view dedicated to
  creating, destroying, and editing any number of attributes that
  share some common base definition (such as a material or boundary
  condition).

Each of these views inherits :smtk:`smtk::extension::qtBaseAttributeView`
and displays items of the attribute.

Items displayed inside these views all inherit `smtk::extension::qtItem`.

Qt view classes also provide facilities that adapt SMTK's
:ref:`observers-pattern` to Qt's signals-and-slots system.
When an item indicates it has modified its parent attribute
(via a Qt signal), the view creates and launches a
:smtk:`smtk::attribute::Signal` operation to notify
core SMTK objects of the change.
Thus, if you are implementing a custom Qt item class, you
should emit a ``modified()`` signal; the parent attribute-view
will consume this signal and run the operation for you.

Likewise, subclasses of `:smtk::extension::qtItem` should not
observe SMTK operations themselves but instead rely on the
Qt subsystem to observe operations. Each time an operation
completes, the Qt view observes it and calls the
``updateItemData()`` on each of its attributes' items.
