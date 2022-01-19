Fixed Crash when loading in multiple attribute resources
--------------------------------------------------------

Closing a CMB application that has more than one attribute resource loaded would cause the application to crash (even if all but one of the attribute resources were closed manually).  This commit fixes the problem by checking to make sure the attribute panel has not been deleted when observing changes.

Added the ability to set the View and Attribute Resource
--------------------------------------------------------
Both displayResource and displayResourceOnServer methods now take in an optional view parameter.  If the view is set, then it will be used to display the resource, else the resource's top level view will be used.
