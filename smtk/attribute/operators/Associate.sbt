<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "associate" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="associate to attribute"
            Label="Attribute - Associate Resource" BaseType="operation">
      <BriefDescription>
        Associate a resource to an attribute resource.
      </BriefDescription>
      <DetailedDescription>
        &lt;p&gt;Associate a resource to an attribute resource.
        &lt;p&gt;The visualized lists of attribute component items are
        populated from associated resources.
      </DetailedDescription>
      <AssociationsDef Name="attribute">
        <Accepts><Resource Name="smtk::attribute::Resource"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>

        <Resource Name="associate to" Label="associated resource(s)"
            NumberOfRequiredValues="1" Extensible="true">
          <Accepts>
            <Resource Name="smtk::resource::Resource"/>
          </Accepts>
        </Resource>

      </ItemDefinitions>
    </AttDef>
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(associate to attribute)" BaseType="result"/>

  </Definitions>
</SMTK_AttributeResource>
