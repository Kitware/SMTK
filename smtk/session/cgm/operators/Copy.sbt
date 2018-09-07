<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CGM "Copy" Operation -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operation -->
    <AttDef Type="copy" BaseType="operation">
      <AssociationsDef Name="Workpiece(s)" NumberOfRequiredValues="1" Extensible="true">
        <Accepts><Resource Name="smtk::session::cgm::Resource" Filter="model|cell|anydim"/></Accepts>
      </AssociationsDef>
      <BriefDescription>
        Copy the entity.
      </BriefDescription>
      <DetailedDescription>
        Make a copy of the given entity.

        Note that CGM treats bodies differently than other entities.
      </DetailedDescription>
      <ItemDefinitions>
        <!-- No parameters. Either you want a copy or you don't. -->
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(copy)" BaseType="result">
      <!-- The translated entities are stored in the base result's "created" item. -->
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
