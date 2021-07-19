.. highlight::cpp

Added the ability to ignore an item
===================================
There are times when a workflow may consider an item no longer relevant based on choices the user has made.  In order model this behavior two methods have been added to Item:

.. code-block:: c++

  void setIsIgnored(bool val);
  bool isIgnored() const;


If setIsIgnored is passed true, then the item's isRelevant() method will return false, regardless of any other facts.
This value is persistent and is supported in both JSON and XML formats.

Changes to Attribute and Item isRelevant Methods
================================================

Both Attribute::isRelevant and Item::isRelevant have been modified to optional do advance level checking.

.. code-block:: c++

  bool isRelevant(bool includeReadAccess = false, int readAccessLevel = 0) const;

If includeReadAccess is set to true then an Attribute is relevant iff at least one of it's children has a read access level <= readAccessLevel as well as passing the existing category checks.

In the case of an Item, if includeReadAccess is true then it must pass both category checks and have it's read access level <= readAccessLevel

Note that this modification does not require any code change in order to preserve previous functionality.

Added hasRelevantChildren method to GroupItem
=============================================
GroupItem now has a method to test  if at least on of its children items passes their category checks (and optionally their advance level checks).

.. code-block:: c++

  bool hasRelevantChildren(bool includeReadAccess = false, int readAccessLevel = 0) const;
