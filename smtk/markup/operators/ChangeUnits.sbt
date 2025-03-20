<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the markup resource's "create" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="change units" Label="change units" BaseType="operation">
      <AssociationsDef LockType="Write">
        <Accepts>
          <!-- Accept any spatial markup component (though only discrete handled now) -->
          <Resource Name="smtk::markup::Resource" Filter="smtk::markup::SpatialData"/>
        </Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="source units" Label="source units">
          <BriefDescription>
            The length units the geometry is specified in; the geometry
          </BriefDescription>
          <DetailedDescription>
            The length units the geometry is specified in; the geometry
            will be converted from this length unit to the resource's default length units.
            Examples include "mm", "meter", "inch", "foot", "yard", or even "furlong".
          </DetailedDescription>
        </String>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(set name)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeResource>
