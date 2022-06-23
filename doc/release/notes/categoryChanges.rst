Expanding SMTK Attribute Category Mechanism
-------------------------------------------

Category Modeling in SMTK Attribute Resources has been enhanced to now support specialization as well as generalization.
Previously, an Attribute or Item Definition's local categories could only expand (make more general) the categories it was inheriting (or choose to ignore them all together).
With this release, SMTK now supports specializing (or making the category constraint more restrictive).

Previously category inheritance was controlled by using the Definition's setIsOkToInherit method which would either **Or** its local categories with those it was inheriting or **Replace** them. This method (as well as isOkToInherit method) has been deprecated.  The new methods for setting and retrieving category inheritance are:

* setCategoryInheritanceMode
* categoryInheritanceMode

The values that categoryInheritanceMode can be set to are:

* smtk::attribute::Categories::CombinationMode::Or
* smtk::attribute::Categories::CombinationMode::And
* smtk::attribute::Categories::CombinationMode::LocalOnly

Setting the mode to **Or** is the same as the previous setIsOkToInherit(true) - it will **or** the Definition's local categories with those that it is inheriting. Setting the mode to **And** will now **and** the Definition's local categories with those that it is inheriting.  Setting the mode to **LocalOnly** will ignore the categories that the Definition is inheriting and is the same as the previous setIsOkToInherit(false).

Changing the Default Category Inheritance
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Previously the default mode was isOkToInherit = true which now corresponds to smtk::attribute::Categories::CombinationMode::Or.  Upon discussing the new combination support of **And**, it was decided that the new default will be smtk::attribute::Categories::CombinationMode::And, meaning that a Definition will be more specialized category-wise than it's parent Definition.

Change on Enum Categories
~~~~~~~~~~~~~~~~~~~~~~~~~
Previously, categories on a Discrete Value Item Definition's Enums would add their categories to the Definition.  With this release it is assumed that the enums' categories will be **and'd** with it's Definition, there is no longer a reason for the enum categories to be combined with the Definition's local categories.

File Version Changes
~~~~~~~~~~~~~~~~~~~~
Supporting these changes did require a new format for both Attribute XML and JSON files.  The latest versions for both is now **6**.  Older file formats will load in properly and should work based on the previous category rules.

Developer changes
~~~~~~~~~~~~~~~~~~

The following methods and enums have been deprecated:

* smtk::attributeCategories::Set::CombinationMode::Any -> please use smtk::attributeCategories::Set::CombinationMode::Or
* smtk::attributeCategories::Set::CombinationMode::All -> please use smtk::attributeCategories::Set::CombinationMode::And
* smtk::attribute::Definition::setIsOkToInherit -> please use smtk::attribute::Definition::setCategoryInheritanceMode
* smtk::attribute::Definition::isOkToInherit -> please use smtk::attribute::Definition::categoryInheritanceMode
* smtk::attribute::ItemDefinition::setIsOkToInherit -> please use smtk::attribute::ItemDefinition::setCategoryInheritanceMode
* smtk::attribute::ItemDefinition::isOkToInherit -> please use smtk::attribute::ItemDefinition::categoryInheritanceMode

A new class for supporting the new combination modes has been developed called smtk::attribute::Categories::Stack which represents the category expression formed when combining inherited and local categories since we now need to maintain the order in which they are combined.

The method smtk::attribute::ValueItem:relevantEnums(bool includeCategories, bool includeReadAccess, unsigned int readAccessLevel) const was added in order to return the set of enums that passed the activce category and advance level checks (if specified).
