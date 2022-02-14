Added API to Disable Category Inheritance for Definitions
---------------------------------------------------------

Added the ability to turn off inheriting category information from the Base Definition. The mechanism to control this is identical to that used at the Item Definition level via the method setIsOkToInherit. This information is persistent and stored in both XML and JSON formats.

Developer changes
~~~~~~~~~~~~~~~~~~

New API:

* bool isOkToInherit() const;
* void setIsOkToInherit(bool isOkToInheritValue);
