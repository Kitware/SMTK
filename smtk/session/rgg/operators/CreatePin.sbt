<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the RGG "CreatePin" Operator -->
<SMTK_AttributeResource Version="3">
  <Definitions>
    <!-- Operator -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="create pin" Label="Model - Create Pin" BaseType="operation">
      <BriefDescription>Create a RGG Pin.</BriefDescription>
      <DetailedDescription>
        By providing a name and label user can create a simple pin.
        After the creation, CMB would automatically switch to "Edit pin" operator
        so that user can tweak other properties.
      </DetailedDescription>
      <AssociationsDef Name="model" NumberOfRequiredValues="1" AdvanceLevel="0">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="name" NumberOfRequiredValues="1" AdvanceLevel="0">
          <BriefDescription>A user assigned name for the nuclear pin</BriefDescription>
          <DetailedDescription>
            A user assigned name for the nuclear pin.
          </DetailedDescription>
          <DefaultValue>Pin0</DefaultValue>
        </String>
        <String Name="label" NumberOfRequiredValues="1" AdvanceLevel="11">
          <BriefDescription>A user assigned label for the nuclear pin</BriefDescription>
          <DetailedDescription>
            A user assigned label for the nuclear pin.
          </DetailedDescription>
          <DefaultValue>PC0</DefaultValue>
        </String>
        <Int Name="cell material" NumberOfRequiredValues="1" AdvanceLevel="11">
          <BriefDescription>A user assigned outer material for the nuclear pin</BriefDescription>
          <DetailedDescription>
            A user assigned outer material for the nuclear pin.
          </DetailedDescription>
          <!-- no material -->
          <DefaultValue>0</DefaultValue>
        </Int>
        <Double Name="color" NumberOfRequiredValues="0" AdvanceLevel="11">
          <!-- When reading from a file, it's allowed to assign a color at creation time-->
        </Double>
        <Void Name="cut away" NumberOfRequiredValues="1" Optional = "true" IsEnabledByDafault="false" AdvanceLevel="11">
          <BriefDescription>cut away the pin so that the inner layers are visible</BriefDescription>
          <DetailedDescription>
            If enabled, SMTK would use a clipping plane that is perpendicular to the bottom and goes through the base radius
            to cut the pin. By doing so, user is able to view the inner layers.
          </DetailedDescription>
        </Void>
        <Double Name="z origin" NumberOfRequiredValues="1" AdvanceLevel="11">
          <BriefDescription>A user assigned z origin for the nuclear pin</BriefDescription>
          <DetailedDescription>
            A user assigned z origin for the nuclear pin.
          </DetailedDescription>
          <DefaultValue>0.0</DefaultValue>
        </Double>
        <Group Name="pieces" Extensible="true" NumberOfRequiredGroups="1" AdvanceLevel="11">
          <BriefDescription>A user assigned a set of sections which form the the nuclear pin from bottom to top</BriefDescription>
          <DetailedDescription>
             A user assigned a set of sections which form the the nuclear pin from bottom to top
          </DetailedDescription>
          <ItemDefinitions>
            <Int Name="segment type" NumberOfRequiredValues="1" AdvanceLevel="11">
              <DiscreteInfo DefaultIndex="0">
                <Value Enum="Cylinder">0</Value>
                <Value Enum="Frustum">1</Value>
              </DiscreteInfo>
            </Int>
            <Double Name="type parameters" NumberOfRequiredValues="3" AdvanceLevel="11">
              <!-- length, base raidus, top radius -->
              <DefaultValue>10 ,0.5, 0.5</DefaultValue>
            </Double>
          </ItemDefinitions>
        </Group>
        <Group Name="layer materials" Extensible="true" NumberOfRequiredGroups="1" AdvanceLevel="11">
          <BriefDescription>A user assigned a set of materials which form the inner layers of the nuclear pin</BriefDescription>
          <DetailedDescription>
             A user assigned a set of materials which form the inner layers of the nuclear pin.
          </DetailedDescription>
          <ItemDefinitions>
            <Int Name="sub material" NumberOfRequiredValues="1" AdvanceLevel="11">
              <BriefDescription>A user assigned material for a nuclear pin layer</BriefDescription>
              <DetailedDescription>
                A user assigned material for a nuclear pin layer
              </DetailedDescription>
              <DefaultValue>0</DefaultValue>
            </Int>
            <Double Name="radius(normalized)" NumberOfRequiredValues="1" AdvanceLevel="11">
              <DefaultValue>1</DefaultValue>
            </Double>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(create pin)" BaseType="result">
      <ItemDefinitions>
        <!-- The created pin is returned in the base result's "create" item. -->
        <Void Name="force camera reset" Optional="true" IsEnabledByDefault="true" AdvanceLevel="11"/>
        <Void Name="hide other entities" Optional="true" IsEnabledByDefault="true" AdvanceLevel="11"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
