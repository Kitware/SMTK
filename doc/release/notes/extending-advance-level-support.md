## Extending Advance Level Support

A full description on how the new advance level support works can be found [here] (https://discourse.kitware.com/t/supporting-advance-read-and-write-at-the-attribute-level/346)

For examples, see attribute/testing/cxx/unitAdvanceLevelTest.cxx and data/attribute/attribute_collection/unitAttributeAdvanceLevelTest.sbi

* Advance Level Information can now be inherited by a Definition from its base Definition
* Advance Level Information can now be inherited by an Item Definition from its Attribute Definition
* Advance Level Information can now be inherited by an Item Definition from its owning Item Definition such as a Group Item Definition and/or Value Item Definition
* Advance Level of an Item is now also based on the advance level of its owning Item or owning Attribute
* **Advance Levels are know represented as unsigned integers instead of signed integers**

### Local Advance Level Information
Attribute Definition, Item Definition, Attribute and Item now have the ability to have local advance level information for both GUI read and write access.  Related methods include:

* setLocalAdvanceLevel: **note that this replaces setAdvanceLevel methods**
* unsetLocalAdvanceLevel
* hasAdvanceLevelInfo

Local advance level information "overrides" the information that would be inherited. For example setting local advance read level for a Definition will override the advance read level that would have been inherited from its base Definition.  Similarly setting the local advance write level for an Attribute will override the advance write level that would have been inherited from its Definition.

### Item's advanceLevel
An Item's Advance Level is now the max of its local level (or if not set, it's definition) and the advance level of its owning Item or owning Attribute.

### Applying Advance Levels
Definition and ItemDefinition has methods for pushing its advance level information to their children Definitions and Item Definitions.  These method are used by Resource's finalizeDefinitions method.

### XML and JSON Support
The same format used for setting local Advance Levels for Items is used for Attribute, Definition, and Item Definition

``` xml
    <AttDef Type="A" Label="A" BaseType="" Version="0" AdvanceWriteLevel="1" Unique="false"\>

```

### Qt View Changes
* qtItem widgets now check to see if they are writable based on their Item's advance write level and the current advance level.  If they are not or if their ItemView has the readonly property, they will be readonly.
* If the current advance level is below an attribute's advance write level, then the attribute will not be eligible for deletion in an AttributeView.
