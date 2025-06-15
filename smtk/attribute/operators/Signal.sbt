<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "signal" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>

    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="signal"
            Label="Attribute - Signal Changes" BaseType="operation">
      <BriefDescription>
        Indicate that an attribute was created, modified, or expunged.
      </BriefDescription>
      <ItemDefinitions>
        <Component Name="created" Extensible="true" NumberOfRequiredValues="0">
          <Accepts><Resource Name="smtk::attribute::Resource" Filter="*"/></Accepts>
        </Component>
        <Component Name="modified" Extensible="true" NumberOfRequiredValues="0">
          <Accepts><Resource Name="smtk::attribute::Resource" Filter="*"/></Accepts>
        </Component>
        <Resource  Name="categoriesModified" Extensible="true" NumberOfRequiredValues="0">
          <Accepts><Resource Name="smtk::attribute::Resource"/></Accepts>
        </Resource>
        <String Name="items" Extensible="true" NumberOfRequiredValues="0"/>
        <String Name="source">
          <DefaultValue></DefaultValue>
        </String>
        <Component Name="expunged" Extensible="true"
                   NumberOfRequiredValues="0" HoldReference="true">
          <Accepts><Resource Name="smtk::attribute::Resource" Filter="*"/></Accepts>
        </Component>
        <Void Name="update" Optional="True"/>
        <Resource  Name="resourcesCreated" Extensible="true" NumberOfRequiredValues="0">
          <Accepts><Resource Name="smtk::attribute::Resource"/></Accepts>
        </Resource>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(signal)" BaseType="result">
        <ItemDefinitions>
          <String Name="items" Extensible="true" NumberOfRequiredValues="0"/>
          <Void Name="update"  Optional="True"/>
          <Resource  Name="categoriesModified" Extensible="true" NumberOfRequiredValues="0">
            <Accepts><Resource Name="smtk::attribute::Resource"/></Accepts>
          </Resource>
        </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
