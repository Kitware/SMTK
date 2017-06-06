<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB Discrete Model "Split Face" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="split face" BaseType="operator">
      <BriefDescription>
        Split a face/faces into several small faces. Feature angle is used to decide whether split the
        face or not.
      </BriefDescription>
      <DetailedDescription>
        Split a face/faces into several small faces. Feature angle is used to decide whether split the
        face or not.

        Only when the feature angle of two adjacent faces is smaller than the threthold provided by the
        user then would the face be split.

      </DetailedDescription>
      <ItemDefinitions>
        <ModelEntity Name="model" NumberOfRequiredValues="1">
          <MembershipMask>model</MembershipMask>
        </ModelEntity>
        <ModelEntity Name="face to split" NumberOfRequiredValues="0" Extensible="1">
        <MembershipMask>face</MembershipMask>
        </ModelEntity>
        <Double Name="feature angle" NumberOfRequiredValues="1">
          <DefaultValue>15.0</DefaultValue>
          <RangeInfo>
            <Min Inclusive="true">0.</Min>
            <Max Inclusive="true">360.</Max>
          </RangeInfo>
           <BriefDescription>
             Feature angle is the angle between the normals of two adjacent faces. Here feature
             angle is defind as an upper bound.
           </BriefDescription>
           <DetailedDescription>
             Feature angle is the angle between the normals of two adjacent faces. Here feature
             angle is defined as an upper bound.

             Feature angle also is the supplementary angle of the dihedral angle between
             two adjacent faces.
           </DetailedDescription>
        </Double>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(split face)" BaseType="result">
      <ItemDefinitions>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeSystem>
