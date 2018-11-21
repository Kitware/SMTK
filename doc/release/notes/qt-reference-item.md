## Expose qtReferenceItem for associations

Now qtReferenceItem (the base class for qtComponentItem and qtResourceItem)
can be instantiated and used in addition to its subclasses.
While the subclasses may choose to behave differently, it was necessary
to make qtReferenceItem available so that operation associations (which
are forced to be ReferenceItems in the attribute system so that operations
may take either resources or components) can be visualized and edited.

The qtReferenceItem (and thus its subclasses) now have improved usability:
+ they highlight their members in the 3-d view on hover;
+ they provide controls to copy their members to/from the application selection
  as well as a button to reset the item;
+ the popup for selecting members via a list is now attached to the item rather
  than appearing as an application-wide modal dialog;
+ they provide immediate visual feedback when the members are edited via
  the popup; and
+ they accept valid edits once the user clicks outside of the popup
  while allowing users to abandon edits with the escape key.
