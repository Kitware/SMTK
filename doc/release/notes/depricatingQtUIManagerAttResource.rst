Changes to qtBaseAttributeView
------------------------------

In the past classes derived from qtBaseAttributeView relied on the qtUIManager to get access to the Attribute Resource they depended on.  This is no longer the case.  The Attribute Resource is now part of the information used to defined the instance.

Deprecation of qtUIManager::attResource() method
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This method is no longer needed for qtBaseAttributeView derived classes.

Added qtItem::attributeResource() method
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This is a convenience method for accessing the qtItem's underlying Attribute Resource
