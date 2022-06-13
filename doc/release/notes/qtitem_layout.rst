qtItem changes
---------------

Several changes were made to the `qtItem` subclasses.

1\. The new code hides the QLabel instance for an item if its label is
set to a (nonempty) blank string. Template writers use a whitespace
character to hide the label, however, a QFrame is still displayed and
takes up horizontal space. The new code essentially removes that unwanted
space.

2\. The new code changes the Qt alignment for horizontal child-item layouts
from vertical-center to top, for aesthetic reasons.

3\. The new code updates the logic for setting the layout direction
(horizontal or vertical) in `qtInputsItem` for various situations.

  * For ordinary value items, the default layout is horizontal, but can be
    overridden by an ItemView `<View Layout="Vertical">`.
  * For extensible items, the layout is *always* vertical, and cannot be
    overridden by an ItemView.
  * For discrete value items with children items, the layout is either:
    (a) horizontal, if each discrete value is assigned no child items or a
    single child item having a blank string for its label; otherwise
    (b) vertical. The discrete item layout can be overridden by an item
    view, either `<View Layout="Horizontalal">` or
    `<View Layout="Vertical">`.
