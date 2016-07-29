<?xml version="1.0" encoding="utf-8" ?>
<!-- Description of the CMB polygon Model "import" Operator -->
<SMTK_AttributeSystem Version="2">
  <Definitions>
    <!-- Operator -->
    <AttDef Type="import image" BaseType="operator">
      <AssociationsDef Name="model" NumberOfRequiredValues="1" Extensible="yes">
        <MembershipMask>model</MembershipMask>
        <BriefDescription>The model that the image will be attached to.</BriefDescription>
        <DetailedDescription>
          The model that the image will be attached to.

          You must not specify both a model and an image file name.
        </DetailedDescription>
      </AssociationsDef>
      <ItemDefinitions>
        <File Name="filename" Label="File Name" NumberOfRequiredValues="1"
          ShouldExist="true"
          FileFilters="Image files (*.vti *.tif);;DEM(*.dem);;All files (*.*)">
        </File>

      </ItemDefinitions>
    </AttDef>
    <!-- Result -->
    <AttDef Type="result(import image)" BaseType="result"/>
  </Definitions>
</SMTK_AttributeSystem>
