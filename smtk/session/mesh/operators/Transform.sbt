<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the Mesh Session "Transform" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="transform" Label="Model - Transform" BaseType="operation">
      <AssociationsDef Name="Model" NumberOfRequiredValues="1" Extensible="false">
        <Accepts><Resource Name="smtk::session::mesh::Resource" Filter="model"/></Accepts>
      </AssociationsDef>
     <ItemDefinitions>
        <Double Name="scale" NumberOfRequiredValues="3">
          <BriefDescription>
            Scale model? If so, specify a scale factor for each axis.
          </BriefDescription>
          <DetailedDescription>
            Enabling this item allows you to specify a scale factor per axis for the
            model.
            Scaling is performed about the origin before rotation and translation.
          </DetailedDescription>
          <DefaultValue>1, 1, 1</DefaultValue>
        </Double>
        <Double Name="rotate" NumberOfRequiredValues="3">
          <BriefDescription>
            Rotate model? If so, specify angles about each axis in degrees.
          </BriefDescription>
          <DetailedDescription>
            Enabling this item allows you to specify angles (in degrees) about which to rotate
            the model. Angles are specified about the origin and rotation is applied
            before translation.
          </DetailedDescription>
          <DefaultValue>0, 0, 0</DefaultValue>
        </Double>
        <Double Name="translate" NumberOfRequiredValues="3">
          <BriefDescription>Translate model? If so, specify a vector.</BriefDescription>
          <DetailedDescription>
            Enabling this item allows you to specify a vector to add to each original point
            of the model.
            Translation is applied after scaling and rotation;
            therefore the vector is not modified by the specifed scaling and rotation (if any).
          </DetailedDescription>
          <DefaultValue>0, 0, 0</DefaultValue>
        </Double>
        </ItemDefinitions>
     </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(transform)" BaseType="result">
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
