<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>

    <!-- Parameters -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="create analytic shape" Label="new analytic shape" BaseType="operation">

      <AssociationsDef LockType="Write" OnlyResources="true" NumberOfRequiredValues="1">
        <BriefDescription>The resource to add the shape to.</BriefDescription>
        <Accepts><Resource Name="smtk::markup::Resource"/></Accepts>
      </AssociationsDef>

      <ItemDefinitions>

        <String Name="shape" Label="shape">
          <BriefDescription>The type of shape to add.</BriefDescription>
          <ChildrenDefinitions>
            <Double Name="box center" Label="center" NumberOfRequiredValues="3"/>
            <Double Name="box size" Label="half-widths" NumberOfRequiredValues="3"/>
            <Double Name="sphere center" Label="center" NumberOfRequiredValues="3"/>
            <Double Name="sphere radius" Label="radii" NumberOfRequiredValues="3"/>
          </ChildrenDefinitions>
          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="box">smtk::markup::Box</Value>
              <Items>
                <Item>box center</Item>
                <Item>box size</Item>
              </Items>
            </Structure>
            <Value Enum="cone">smtk::markup::Cone</Value>
            <Value Enum="plane">smtk::markup::Plane</Value>
            <Structure>
              <Value Enum="sphere">smtk::markup::Sphere</Value>
              <Items>
                <Item>sphere center</Item>
                <Item>sphere radius</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>

      </ItemDefinitions>

    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(create analytic shape)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>

  </Definitions>
</SMTK_AttributeResource>
