Common and Attribute
====================

Category Support has moved from attribute to common
---------------------------------------------------

All category related functionality has been removed from smtk/attribute and is now in smtk/common.
This was done so that other SMTK classes such as Task Worklets, Task Managers, and Tasks themselves can
make use of this functionality.

**Note** - This also means that Categories' namespace has also changes and will require code modification though the change is trivial.  Simply replace smtk::attribute::Categories with smtk::common::Categories.

The same is true for code supporting category-based expression grammars.
