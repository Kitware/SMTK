## Category Changes to Attribute Resource

* Categories can now be assigned to Attribute Definitions and Group Item Definitions
  * A Category assigned to an Attribute Definition, Value Item Definition, or Group Item Definition is also (by default) inherited by their child item definitions.  This will simplify  creating template files since the author will now be able to state that attributes (and their items) are of category "X" with a single command/specification instead of having to explicitly  assign "X" to all of the item definitions associated with the attribute definition.
  * Template authors will no longer have to be careful to specify categories to optional children items of a value item since they can now inherit those assigned to the value item itself.
* Categories assigned explicitly to an attribute definition or item definition are now referred to as **Local Categories**. The set of categories (both explicit and inherited) are referred to as **Categories**.
  * **smtk::attribute::ItemDefinition::addCategory(...) has been replaced**. The new method is mtk::attribute::ItemDefinition::addLocalCategory(...)
  * **smtk::attribute::ItemDefinition::removeCategory(...) has been replaced**. The new method is mtk::attribute::ItemDefinition::removeLocalCategory(...)
  * A new method **smtk::attribute::ItemDefinition::localCategories()** has been added.  This returns all categories explicitly assigned to the item definition.	* You can control whether an item definition should inherit the categories from its owning attribute definition and Item Definition (in the case of Group or Value Item children)
    *  **smtk::attribute::ItemDefinition::isOkToInherit()** returns true if it's ok to inherit categories from its parent.  **Default is true**
    *  **smtk::attribute::ItemDefinition::setIsOkToInherit(bool)** for setting the item definition's category inheritance behavior
  * Added methods to attribute::Definition for specifying its local categories
    * **smtk::attribute::Definition::addLocalCategory(...)** - adds a local category to the Definition
    * **smtk::attribute::Definition::removeLocalCategory(...)** - removes a local category from the Definition
    * **smtk::attribute::Definition::localCategories()** - returns the local categories assigned to the Definition

* **Definition and Item Definition updateCategories() method have been replaced**. The new methods are:
  * attribute::Definition::applyCategories(...) and attribute::ItemDefinition::applyCategories(...)

### Inheritance Rules
* A Definition will inherit all of the local categories associated with its Item Definitions (and their descendants) along with all of the categories associated with it's base definition
* An Item Definition will inherit all of local categories associated with it's children item definitions (and their descendants) and if it's isOkToInherit mode is true it will also inherit all local categories assigned to it's parent (and their ancestor up to and including the first ancestor whose isOkToInherit mode is false).  Note that this could include local categories associated with it's owning attribute Definition and it's base definition

### Example

Consider the following scenario where (...) denotes a local category

* Definition A (A)
  * Group Item Definition g1 (g1)
     * String Item Definition s1 (s1)
         * String Item Definition s2 (s2) - isOkToInherit (false)
             * Void Item Definition v1 (v1)
     * String Item Definition s3 (s3) - isOkToInherit (false)
* Definition B (B) - base definition is A
  * Void Item Definition v2 (v2)

The resulting categories would be:

* A: A, g1, s1, s2, v1, s3
* B: A, g1, s1, s2, v1, s3, B, v2
* g1: A, g1, s1, s2, v1, s3 (same as A)
* s1: A, g1, s1, , s2, v1
* s2: s2, v1
* s3: s3
* v1: s2, v1
* v2: A, B, v2
