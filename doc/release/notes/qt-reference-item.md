## Expose qtReferenceItem for associations

Now qtReferenceItem (the base class for qtComponentItem and qtResourceItem)
can be instantiated and used in addition to its subclasses.
While the subclasses may choose to behave differently, it was necessary
to make qtReferenceItem available so that operation associations (which
are forced to be ReferenceItems in the attribute system so that operations
may take either resources or components) can be visualized and edited.
