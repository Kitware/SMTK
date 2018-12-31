#Adding Exclusions and Prerequisites
Attribute Definitions can now provide mechanisms for modeling exclusion and prerequisites constraints.  An exclusion constraint prevents an attribute from being associated to same persistent object if another attribute is already associated and its definition excludes the attribute's type.  For example, consider two attribute definitions A and B as well as two attributes a (of type A) and b (or type B).  If we have specified that A and B excludes each other then if we have associated a to an object, we are prevented from associating b to the same object.

In the case of a prerequisite, an attribute is prevent from being associated if there is not an attribute of an appropriate type associated with the object.  So continuing the above example lets assume we add a new definition C that has A as it prerequisite and an attribute c (type C).  c can be associated to an object only if a is already associated to it.

In addition, these properties are also inherited.  So if we derive type A1 from A in the above example, an attribute a1 (from type A1), would excludes b and would be considered a prerequisite of c.

**NOTE - the exclusion property should be symmetric (if A doesn't allow B then B shouldn't allow A) but the prerequisite property can not be symmetric (else neither could be associated - if you need to have both attributes always assigned together then the information should be modeled as a single definition)**

To implement this, the following was added to Definition:

* Set of excluded definitions
* Set of prerequisite definitions
* Added methods to add and remove definitions - when adding an exclusion, it is done symmetrically. So calling A->addExclusion(B) will result in B being added to A's exclusion set and A being added to B's exclusion's set.
* Added methods for checking association rules, exclusions, and prerequisites
* Added canBeAssociated method for testing persistent objects
* I/O classes can read and write these rules in XML and JSON


In addition, the implementation of isUnique has been changed  It now used the exclusion mechanism by inserting
itself into the list.  Note that this rule is not written out when serialized or when saved
to a file


The qtAssociationWidget has been modified to use these new ruled when determining availability and qtAttributeView has been modified to test for association rules instead of association masks


See attribute/testing/cxx/unitAttributeAssociationConstraints.cxx for an example.
