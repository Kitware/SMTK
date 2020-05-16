<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the OpenCASCADE "box" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="create box" Label="Model - Create Box" BaseType="operation">

      <!-- This operation can use an existing resource if one is
           provided. Otherwise, a new resource is created -->
      <AssociationsDef Name="add to resource" NumberOfRequiredValues="0"
                       Extensible="true" MaxNumberOfValues="1" OnlyResources="true">
        <Accepts><Resource Name="smtk::session::opencascade::Resource"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>
        <Double Name="center" Label="Center" NumberOfRequiredValues="3">
          <BriefDescription>
            The coordinates of the center of the box.
          </BriefDescription>
          <DetailedDescription>
            The coordinates of the center of the box.
          </DetailedDescription>
          <DefaultValue>0.0,0.0,0.0</DefaultValue>
        </Double>

        <Double Name="size" Label="Size" NumberOfRequiredValues="3">
          <BriefDescription>
            The extent of the box, along each axis, to either side of its center.
          </BriefDescription>
          <DetailedDescription>
            The extent of the box, along each axis, to either side of its center.
            Zero and negative values are not allowed.
          </DetailedDescription>
          <DefaultValue>1.0,1.0,1.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="false">0.0</Min>
          </RangeInfo>
        </Double>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(create box)" BaseType="result">
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
