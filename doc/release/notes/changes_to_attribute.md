## Changes to Attribute
### ItemDefinitionManager: remove requirement for resource manager

Originally, ItemDefinitionManager acted on resources indirectly through
the resource manager. The new design can still associate to a resource
manager and augment its attribute resources, but it is also functional as
a standalone manager and can add custom item definitions to attribute
resources directly. This change removes the requirement for attribute's
Read and Import operations to construct attribute resources using the
resource manager, so attribute resources can now be populated prior to
being added to the resource manager.
