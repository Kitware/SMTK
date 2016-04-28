<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the smtk polygon Model "edit edge" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="edit edge" BaseType="operator">
      <AssociationsDef Name="model" NumberOfRequiredValues="1" Extensible="yes">
        <MembershipMask>model</MembershipMask>
        <BriefDescription>The model to which this edge op will be operated on.</BriefDescription>
        <DetailedDescription>
          The model to which this edge op (create, remove, edit edge) will be operated on.
        </DetailedDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="Operation" Label="Operation" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
          <BriefDescription>operation for the operator</BriefDescription>
          <ChildrenDefinitions>
            <ModelEntity Name="edit edge" NumberOfRequiredValues="1">
              <MembershipMask>edge</MembershipMask>
            </ModelEntity>
            <ModelEntity Name="remove edge" Extensible="1" NumberOfRequiredValues="0">
              <MembershipMask>edge</MembershipMask>
            </ModelEntity>

            <String Name="edge name" Label="edge name" Version="0" AdvanceLevel="0" NumberOfRequiredValues="1">
              <DefaultValue>new edge</DefaultValue>
            </String>

            <Double Name="edge points" NumberOfRequiredValues="6" Extensible="yes">
              <BriefDescription>The (x,y,z) coordinates of the edges.</BriefDescription>
              <DetailedDescription>
                The world coordinates of 1 or more edges.
              </DetailedDescription>
            </Double>
            <Int Name="edge offsets" NumberOfRequiredValues="1" Extensible="true">
              <DefaultValue>0</DefaultValue>
              <BriefDescription>Offsets into the list of "edge points" where each edge starts.</BriefDescription>
              <DetailedDescription>
                Offsets into the list of points where each edge starts.

                When "edge points" are specified, each offset value is multiplied by 3.
                Thus, where "points" are passed, one would specify
                offsets equal to "[0, 3, 5]" to indicate the first edge has 3 points,
                the second edge has 2 points, and a third edge exists at the end after these two.
              </DetailedDescription>
            </Int>

          </ChildrenDefinitions>

          <DiscreteInfo DefaultIndex="0">
            <Structure>
              <Value Enum="Create Edge">Create</Value>
              <Items>
                <Item>edge name</Item>
<!--
                <Item>edge points</Item>
                <Item>edge offsets</Item>
-->
              </Items>
            </Structure>
            <Structure>
              <Value Enum="Edit Edge">Edit</Value>
              <Items>
                <Item>edit edge</Item>
              </Items>
            </Structure>
            <Structure>
              <Value Enum="Remove Edge">Remove</Value>
              <Items>
                <Item>remove edge</Item>
              </Items>
            </Structure>
          </DiscreteInfo>
        </String>

        <Int Name="HelperGlobalID" Label="Unique global ID for a helper object" AdvanceLevel="11" NumberOfRequiredValues="1" Optional="true">
          <DefaultValue>0</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(edit edge)" BaseType="result"/>
  </Definitions>

  <Views>
     <!-- The customized view "Type" needs to be the the same as in plugin macro
      ADD_SMTK_UI_VIEW(
        OUTIFACES
        OUTSRCS
        CLASS_NAME qtPolygonEdgeOperationView
        VIEW_NAME smtkPolygonEdgeView
        )
      -->
    <View Type="smtkPolygonEdgeView" Title="Edit Polygon Edge">
      <AttributeTypes>
        <Att Type="edit edge" />
      </AttributeTypes>
    </View>
  </Views>

</SMTK_AttributeSystem>
