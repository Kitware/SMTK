Attribute System
----------------

Definition: Added Ability to Ignore Categories
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can now indicate that an Attribute's or Definition's validity and/or relevance does not depend on the Resource's active categories.  This can be very useful in the case of Definitions and Attributes that model Analyses since they tend to control the set of Active Categories and therefore do not depend on them.

The new methods are:

* Definition::ignoreCategories() const;
* Definition::setIgnoreCategories(bool val);

This information can be specified in SBT files as a XML Attribute called IgnoreCategories and is saved in the Attribute Resource's JSON and XML format.  Python bindings have also been added.
