<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the "edit attribute item" Operation -->
<SMTK_AttributeResource Version="3">
  <Definitions>

    <!-- Operation -->
    <include href="smtk/operation/Operation.xml"/>
    <AttDef Type="edit attribute item" Label="Edit Item" BaseType="operation">
      <BriefDescription>
        Make a change to one item of an attribute.
      </BriefDescription>
      <AssociationsDef Name="attribute">
        <Accepts><Resource Name="smtk::attribute::Resource" Filter="attribute"/></Accepts>
        <BriefDescription>
          The attribute containing the item to be edited.
        </BriefDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <String Name="item path">
          <BriefDescription>
            The path to the item whose value or enabled status will be edited.
          </BriefDescription>
        </String>
        <Int Name="enable" Optional="true" EnabledByDefault="false">
          <BriefDescription>
            If enabled and the item is optional, its state will set to true or false
            depending on the value of this item (positive for true and zero for false).
            It is an error for this to be enabled and its value to be negative.
          </BriefDescription>
          <DefaultValue>-1</DefaultValue>
        </Int>
        <Int Name="extend">
          <BriefDescription>
            If this value is non-negative and the item is extensible, set its number of
            values to the requested size. This may fail depending on the number of values
            required by the item.
          </BriefDescription>
          <DefaultValue>-1</DefaultValue>
        </Int>
        <String Name="value" Extensible="true" NumberOfRequiredValues="0">
          <BriefDescription>
            The vector of values the item should hold.
          </BriefDescription>
          <DetailedDescription>
            The vector of values the item should hold.
            The length of this vector may be zero (if no edits to item values are to be made).
            If of non-zero length, the length must match the item's current size unless the
            "extend" item is non-negative – in which case the length must match that value.
          </DetailedDescription>
        </String>
      </ItemDefinitions>
    </AttDef>

    <!-- Result -->
    <include href="smtk/operation/Result.xml"/>
    <AttDef Type="result(edit attribute item)" BaseType="result">
        <ItemDefinitions>
          <String Name="errors" Extensible="true" NumberOfRequiredValues="0"/>
        </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
