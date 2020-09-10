## Changes to Attribute
### ItemDefinitionManager: remove requirement for resource manager

Originally, ItemDefinitionManager acted on resources indirectly through
the resource manager. The new design can still associate to a resource
manager and augment its attribute resources, but it is also functional as
a standalone manager and can add custom item definitions to attribute
resources directly. This change removes the requirement for attribute's
Read and Import operations to construct attribute resources using the
resource manager, so attribute resources can now be populated prior to
being added to the resource manager.

### Introduce custom association rules

The ability to append custom rules that determine whether an object can be
associated/dissociated to/from an attribute has been added. It is used in
the following way:
1. A custom rule type that inherits from `smtk::attribute::Rule` is either
registered to an attribute resource directly via its Rule factories
(e.g.
```
resource->associationRules().associationRuleFactory().registerType<FooRule>();
resource->associationRules().associationRuleFactory().addAlias<FooRule>("Foo");
```)
or indirectly using the AssociationRuleManager (e.g.
```
void Registrar::registerTo(const smtk::attribute::AssociationRuleManager::Ptr& manager)
{
  manager->registerAssociationRule<FooRule>("Foo");
}
```).
2. An instance of this custom rule type, identified by its alias, can then
be defined in the v4 XML description of an attribute resource:
```
  <AssociationRules>
    <Foo Name="myFoo">
      ...
    </Foo>
  </AssociationRules>
  <DissociationRules>
    ...
  </DissociationRules>
  <Definitions>
    ...
  </Definitions>
</SMTK_AttributeResource>
```
3. Finally, An attribute definition can be associated with the custom
rules:
```
<AttDef Type="att" BaseType="">
  <AssociationRule Name="myFoo"/>
  ...
</AttDef>

```

### Introduce PythonRule

As an example of a custom association/dissociation rule, a PythonRule
has been introduced that accepts a Python snippet describing the rule:
```
<PythonRule Type="smtk::attribute::PythonRule" Name="myPythonRule"><![CDATA[
def canBeAssociated(attribute, object):
    print("can %s be associated to %s? Is it a mesh component?" % \
         ( attribute, object ) )
    import smtk.mesh
    meshComponent = smtk.mesh.Component.CastTo(object)
    return meshComponent != None
  ]]></PythonRule>
```

PythonRule instances expect a Python function that accepts "attribute"
and "object" input parameters to determine whether an object can be
associated/dissociated to/from an attribute. Additionally, external Python
source files describing different modules can be listed using the <SourceFiles>
XML tag. For an example of its use, see `unitAssociationRulesTest.cxx`.
