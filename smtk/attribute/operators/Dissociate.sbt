<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "dissociate" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="associate to attribute"
            Label="Attribute - Dissociate Resource" BaseType="operation">
      <BriefDescription>
        Dissociate a resource from an attribute resource.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Dissociate a resource from an attribute resource.
        &lt;p&gt;The visualized lists of attribute component items are
        populated from associated resources.
      </DetailedDescription>
      <AssociationsDef Name="attribute">
        <Accepts><Resource Name="smtk::attribute::Resource"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>

        <Resource Name="dissociate from" Label="resource(s) to dissasociate"
            NumberOfRequiredValues="1" Extensible="true">
          <Accepts>
            <Resource Name="smtk::resource::Resource"/>
          </Accepts>
        </Resource>

      </ItemDefinitions>
    </AttDef>
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(dissociate from attribute)" BaseType="result"/>

  </Definitions>
</SMTK_AttributeResource>
