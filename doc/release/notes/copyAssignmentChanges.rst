Changes to Copying Attributes and Assigning Attributes and Items
----------------------------------------------------------------

The old smtk::attribute::Resource::copyAttribute method has been deprecated by a
more flexible version that takes in three parameters:

* The Attribute to be copied
* A CopyAndAssignmentOption instance (this is a new class)
* A smtk::io::Logger instance

Much of the attribute "assignment" logic has been moved from the method to the new  smtk::attribute::Attribute::assign(...) method
which as the same signature as the copyAttribute method.

Similarly, the original smtk::attribute::Item::assign method has also been deprecated by a version that takes in the following parameters:

* The SourceItem whose values are to be assigned to the target Item
* A CopyAndAssignmentOption instance (this is a new class)
* A smtk::io::Logger instance

CopyAssignmentOptions class
~~~~~~~~~~~~~~~~~~~~~~~~~~~

This class represents three classes of Options:

* Copy Options controlling how an Attribute gets copied
* Attribute Assignment Options controlling how attribute values are assigned to another
* Item Assignment Options controlling how item values are assigned to another.

AttributeCopyOptions
^^^^^^^^^^^^^^^^^^^^
* copyUUID -  If set, this indicates that copied attributes should have the same UUID as the original.
  **Note** : the copying process will fail if the copied attribute would reside in the same resource as the original.

* copyDefinition - If set, this indicates that if the source attribute's definition (by typename) does not exist in the resource
  making the copy, then copy the definition as well.  This can recursively cause other definitions to be copied.
  **Note** : the copying process will fail if this option is not set and the source attribute definition's typename
  does not exist in the targeted resource.

AttributeAssignmentOptions
^^^^^^^^^^^^^^^^^^^^^^^^^^
* ignoreMissingItems -  If set, this indicates that not all of the source attribute's items must exist in the
  target attribute.  This can occur if the target attribute's definition is a variation of
  the source attribute's.
  **Note** : the assignment process will fail if this option is not set and if not all of the
  source attribute's items are not present in the target.
* copyAssociations - If set, this indicates that the source attribute's associations should be copied
  to the target attribute which will also take into consideration allowPartialAssociations
  and doNotValidateAssociations options.
* allowPartialAssociations - Assuming that copyAssociations option is set, if the allowPartialAssociations
  ** is not set ** then all of the source's associations must be associated
  to the target attribute, else the assignment process will return failure.
* doNotValidateAssociations - Assuming that copyAssociations option is set, the doNotValidateAssociations
  *hint* indicates that if it possible to assign the association information
  without accessing the corresponding persistent object, then do so without
  validation.

ItemAssignmentOptions
^^^^^^^^^^^^^^^^^^^^^
* ignoreMissingChildren - If set, this indicates that not all of the source item's children items must exist in the
  target item.  This can occur if the target item's definition is a variation of the source item's.
  **Note** : the assignment process will fail if this option is not set and if not all of the
  source item's children items are not present in the target.

* allowPartialValues - If set,  this indicates that not all of the source item's values must be
  copied to the target item. If this option ** is not set ** then all of the
  source item's values must be copied, else the assignment process will return failure.

* ignoreExpressions - If set, this indicates that if a source Value item that have been assigned
  an expression attribute, it's corresponding target item should be left unset.

* ignoreReferenceValues - If set, this indicates that a target Reference item should not be assigned
  the values of the corresponding source item.

* doNotValidateReferenceInfo - The doNotValidateReferenceInfo *hint* indicates that if it possible to assign a source Reference item's
  values to a target item without accessing the corresponding persistent object, then do so without validation.

* disableCopyAttributes - If set, this indicates that no attributes should be created when doing item assignments.
  An item assignment can cause an attribute to be created in two situations.

  First - A source Value item is set to an expression attribute that resides in the same
  resource and the target item resides in a different one.  In this case the default
  behavior is to also copy the expression attribute to the target item's resource and
  assign the copied attribute to the target item.

  Second - A source Reference item refers to an attribute that resides in the same
  resource and the target item resides in a different one.  In this case the default
  behavior is to also copy the referenced attribute to the target item's resource and
  assign the copied attribute to the target item.
