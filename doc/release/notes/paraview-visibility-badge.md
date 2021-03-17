## VisibilityBadge fixed to handle multiple resources

It is not an error for a SelectionFootprint query to return
components from other resources (e.g., an assembly resource
might have components that do not contain geometry but instead
reference the geometry of components assembled from other
resources). In this case, we should attempt to set the visibility
of those components in their matching resource's representation.
The VisibilityBadge now does this instead of attempting to set
the visibility of a component in its own representation (which
has no effect since the component does not exist in that scope).
