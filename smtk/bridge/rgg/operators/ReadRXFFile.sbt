<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the model "ReadRXFFile" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="read rxf file" Label="Model - Read RXF File" BaseType="operator">
      <BriefDescription>
        Import an XML description of rgg entities in rxf format.
      </BriefDescription>
      <DetailedDescription>
        Import an XML description of rgg entities in rxf format.
        In CMB5 it requres the user to create a model first. This reader would
        decide core properties(height, thicknesses) based on duct info.
        Due to the limitation of xrf file, geometry type can only be decided
        when assemblies are present.
      </DetailedDescription>
      <AssociationsDef Name="model" NumberOfRequiredValues="1" AdvanceLevel="0">
        <MembershipMask>model</MembershipMask>
      </AssociationsDef>
      <ItemDefinitions>
        <File Name="filename" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="rgg XML file (*.rxf *.xml);;All files (*.*)">
        </File>
      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(read rxf file)" BaseType="result">
      <ItemDefinitions>
        <Void Name="allow camera reset" IsEnabledByDefault="true" AdvanceLevel="11"/>
        <ModelEntity Name="tess_changed" NumberOfRequiredValues="0" Extensible="true"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Views>
     <!--
      The customized view "Type" needs to match the plugin's VIEW_NAME:
      add_smtk_ui_view(...  VIEW_NAME smtkRGGReadRXFFileView ...)
      -->
    <View Type="smtkRGGReadRXFFileView" Title="Read RXF File"  FilterByCategory="false"  FilterByAdvanceLevel="false" UseSelectionManager="false">
      <Description>
        A Hidden view which is used to process xml on the client side.
      </Description>
      <AttributeTypes>
        <Att Type="read rxf file"/>
      </AttributeTypes>
    </View>
  </Views>
</SMTK_AttributeSystem>
