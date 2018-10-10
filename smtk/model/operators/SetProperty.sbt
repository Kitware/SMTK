<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "SetProperty" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="set property" Label="Entities - Set Property" BaseType="operation" AdvanceLevel="10">
      <!-- TODO: When Mesh work is done, set NumberOfRequiredValues="1" -->
      <AssociationsDef Name="Entities" NumberOfRequiredValues="0" Extensible="true">
        <Accepts><Resource Name="smtk::model::Resource"/></Accepts>
      </AssociationsDef>
      <BriefDescription>
        Set (or remove) a property value on a set of entities. Because this op can also
        take MeshEntity as input, so NumberOfRequiredValues for model associatons is 0.
      </BriefDescription>
      <DetailedDescription>
        Set (or remove) a property value on a set of entities.
        The string, integer, and floating-point values are all optional.
        Any combination may be specified.
        All that are specified are set; those unspecified are removed.

        For example, specifying both a string and an integer value for
        the "foo" property would set those values in the model resource's
        string and integer maps while removing "foo" from the associated
        entities' floating-point map.
      </DetailedDescription>
      <ItemDefinitions>
        <MeshEntity Name="meshes" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11">
        </MeshEntity>
        <String Name="name" NumberOfRequiredValues="1">
          <BriefDescription>The name of the property to set.</BriefDescription>
        </String>
        <Double Name="float value" NumberOfRequiredValues="0" Extensible="true">
          <BriefDescription>Floating-point value(s) of the property.</BriefDescription>
        </Double>
        <String Name="string value" NumberOfRequiredValues="0" Extensible="true">
          <BriefDescription>String value(s) of the property.</BriefDescription>
        </String>
        <Int Name="integer value" NumberOfRequiredValues="0" Extensible="true">
          <BriefDescription>Integer value(s) of the property.</BriefDescription>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(set property)" BaseType="result">
      <ItemDefinitions>
        <!-- The modified entities are stored in the base result's "modified" item. -->
        <MeshEntity Name="mesh_modified" NumberOfRequiredValues="0" Extensible="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
