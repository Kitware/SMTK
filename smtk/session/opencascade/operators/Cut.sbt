<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the OpenCASCADE "box" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="cut" Label="Model - Subtract" BaseType="operation">

      <!-- This operation can use an existing resource if one is
           provided. Otherwise, a new resource is created -->
      <AssociationsDef Name="workpieces" NumberOfRequiredValues="1"
                       Extensible="true" >
        <Accepts><Resource Name="smtk::session::opencascade::Resource" Filter="*"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>
        <Component Name="tools" Label="Cutting Tools" NumberOfRequiredValues="1" Extensible="true">
          <BriefDescription>
            The shapes to subtract from the associated workpieces.
          </BriefDescription>
          <DetailedDescription>
            The shapes to subtract from the associated workpieces.
          </DetailedDescription>
          <Accepts><Resource Name="smtk::session::opencascade::Resource" Filter="*"/></Accepts>
        </Component>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(cut)" BaseType="result">
      <ItemDefinitions>

        <!-- The model imported from the file. -->
        <Resource Name="resource" HoldReference="true">
          <Accepts>
            <Resource Name="smtk::session::opencascade::Resource"/>
          </Accepts>
        </Resource>

        <Void Name="allow camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
